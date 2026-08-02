#include "core/triage/triage.h"
#include "persistence/corpus_store.h"

#include <ctime>
#include <csignal>
#include <filesystem>
#include <fstream>
#include <unordered_set>

// ------------------------------------------------------------
//  Triage ctor
// ------------------------------------------------------------
Triage::Triage(EventBus& bus, CrashStore& store, CorpusStore& corpus)
    : bus_(bus)
    , store_(store)
    , corpus_(corpus)
    , last_sig_(0)
    , unique_count_(0)
{}

// ------------------------------------------------------------
//  Logging helper
// ------------------------------------------------------------
void Triage::log_alert(const std::string& msg)
{
    bus_.publish("alert", msg);

    std::filesystem::create_directories("logs");
    std::ofstream out("logs/alerts.log", std::ios::app);
    if (out)
        out << msg << "\n";
}

// ------------------------------------------------------------
//  Stacktrace saving
// ------------------------------------------------------------
void Triage::save_stacktrace(const ExecResult& exec)
{
    if (exec.stderr_log.empty())
        return;

    std::filesystem::create_directories("corpus/crashes");

    const auto ts = std::time(nullptr);
    std::string path = "corpus/crashes/" + std::to_string(ts) + ".stack";

    std::ofstream out(path);
    if (out)
        out << exec.stderr_log;

    log_alert("Stacktrace saved to " + path);
    bus_.publish("alert:stacktrace_saved", path);
}

// ------------------------------------------------------------
//  Hash helper (FNV-1a)
// ------------------------------------------------------------
static uint64_t fnv1a(const std::string& s)
{
    uint64_t h = 0xcbf29ce484222325ULL;
    for (unsigned char c : s) {
        h ^= c;
        h *= 0x100000001b3ULL;
    }
    return h;
}

// ------------------------------------------------------------
//  Crash-key computation
// ------------------------------------------------------------
uint64_t Triage::compute_crash_key(const ExecResult& exec)
{
    uint64_t h = 0xcbf29ce484222325ULL;

    auto mix = [&](uint64_t x) {
        h ^= x;
        h *= 0x100000001b3ULL;
    };

    mix(exec.signal);
    mix(exec.exit_code);
    mix(static_cast<uint64_t>(exec.status));
    mix(fnv1a(exec.stderr_log));

    return h;
}

// ------------------------------------------------------------
//  Old signature (if нужно где-то ещё)
// ------------------------------------------------------------
uint64_t Triage::compute_signature(const ExecResult& exec,
                                   const std::vector<PatternHit>& patterns)
{
    uint64_t h = 0xcbf29ce484222325ULL;

    auto mix = [&](uint64_t x) {
        h ^= x;
        h *= 0x100000001b3ULL;
    };

    mix(exec.exit_code);
    mix(exec.signal);
    mix(static_cast<uint64_t>(exec.status));

    for (const auto& p : patterns) {
        mix(static_cast<uint64_t>(p.kind));
        mix(static_cast<uint64_t>(p.weight * 1000));
        mix(static_cast<uint64_t>(p.position));
    }

    return h;
}

// ------------------------------------------------------------
//  Unique crash detection (по старому sig, если нужно)
// ------------------------------------------------------------
bool Triage::is_unique(uint64_t sig)
{
    if (sig == 0) return false;
    if (sig != last_sig_) {
        last_sig_ = sig;
        unique_count_++;
        return true;
    }
    return false;
}

// ------------------------------------------------------------
//  Crash classification helpers
// ------------------------------------------------------------
bool Triage::is_real_crash(const ExecResult& exec)
{
    switch (exec.signal) {
        case SIGSEGV:
        case SIGILL:
        case SIGBUS:
        case SIGABRT:
            return true;
        default:
            return false;
    }
}

bool Triage::is_timeout(const ExecResult& exec)
{
    return exec.status == ExecStatus::TIMEOUT ||
           exec.signal == SIGKILL;
}

bool Triage::is_js_exception(const ExecResult& exec)
{
    return exec.status == ExecStatus::JS_EXCEPTION &&
           exec.signal == 0 &&
           exec.exit_code != 0;
}

bool Triage::is_sandbox_error(const ExecResult& exec)
{
    return exec.status == ExecStatus::SANDBOX_FAILURE;
}

// ------------------------------------------------------------
//  Severity classification
// ------------------------------------------------------------
int Triage::severity(const ExecResult& exec)
{
    if (is_real_crash(exec))                 return 5; // native crash
    if (is_sandbox_error(exec))             return 4; // sandbox failure
    if (is_timeout(exec))                   return 3; // timeout
    if (exec.status == ExecStatus::INTERESTING) return 2;
    if (is_js_exception(exec))              return 1; // JS exception
    return 0;                                      // OK
}

// ------------------------------------------------------------
//  Main classification logic
// ------------------------------------------------------------
void Triage::classify(const ExecResult& exec,
                      const Script& script,
                      const std::vector<PatternHit>& patterns)
{
    bus_.publish("triage:start", "triage started");

    const uint64_t crash_key = compute_crash_key(exec);
    const int sev = severity(exec);

    // ------------------------------------------------------------
    // Deduplication for severe cases
    // ------------------------------------------------------------
    if (sev >= 3) { // CRASH / SANDBOX / TIMEOUT
        if (seen_keys_.count(crash_key)) {
            bus_.publish("triage:duplicate", "duplicate crash");
            bus_.publish("triage:end", "triage finished");
            return;
        }
        seen_keys_.insert(crash_key);
    }

    // ------------------------------------------------------------
    // REAL CRASH
    // ------------------------------------------------------------
    if (sev == 5) {

        log_alert("Native crash detected");
        bus_.publish("triage:crash", "native crash");

        save_stacktrace(exec);

        CrashEntry entry;
        entry.signature = std::to_string(crash_key);
        entry.original  = script.code;
        entry.minimized = ""; // minimizer заполнит позже
        entry.patterns  = patterns;
        entry.exec      = exec;
        entry.timestamp = std::time(nullptr);

        store_.save(entry);

        corpus_.save_case(script.code,
                          ExecStatus::CRASH,
                          TriageResult::Crash(),
                          patterns);

        bus_.publish("triage:end", "triage finished");
        return;
    }

    // ------------------------------------------------------------
    // SANDBOX FAILURE
    // ------------------------------------------------------------
    if (sev == 4) {

        log_alert("Sandbox failure detected");
        bus_.publish("triage:sandbox_error", "sandbox failure");

        corpus_.save_case(script.code,
                          ExecStatus::SANDBOX_FAILURE,
                          TriageResult::SandboxError(),
                          patterns);

        bus_.publish("triage:end", "triage finished");
        return;
    }

    // ------------------------------------------------------------
    // TIMEOUT
    // ------------------------------------------------------------
    if (sev == 3) {

        log_alert("Timeout detected");
        bus_.publish("triage:timeout", "timeout occurred");

        corpus_.save_case(script.code,
                          ExecStatus::TIMEOUT,
                          TriageResult::Timeout(),
                          patterns);

        bus_.publish("triage:end", "triage finished");
        return;
    }

    // ------------------------------------------------------------
    // INTERESTING CASE
    // ------------------------------------------------------------
    if (sev == 2) {

        log_alert("Interesting case detected");
        bus_.publish("triage:interesting", "interesting case");

        corpus_.save_case(script.code,
                          ExecStatus::INTERESTING,
                          TriageResult::Interesting(),
                          patterns);

        bus_.publish("triage:end", "triage finished");
        return;
    }

    // ------------------------------------------------------------
    // JS EXCEPTION (quiet mode)
// ------------------------------------------------------------
    if (sev == 1) {

        // тихий лог — без alert-спама
        bus_.publish("triage:js_exception", "js exception");

        // НЕ сохраняем в corpus — это шум
        bus_.publish("triage:end", "triage finished");
        return;
    }

    // ------------------------------------------------------------
    // OK
    // ------------------------------------------------------------
    bus_.publish("triage:ok", "no crash");
    bus_.publish("triage:end", "triage finished");
}

// ------------------------------------------------------------
//  Reset for HuntLoop::restart()
// ------------------------------------------------------------
void Triage::reset()
{
    last_sig_ = 0;
    unique_count_ = 0;
    seen_keys_.clear();
}
