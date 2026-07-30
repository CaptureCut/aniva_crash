#include "core/triage/triage.h"
#include <ctime>

Triage::Triage(EventBus& bus, CrashStore& store)
    : bus_(bus)
    , store_(store)
{}

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

bool Triage::is_unique(uint64_t sig) {
    if (sig == 0) return false;
    if (sig != last_sig_) {
        last_sig_ = sig;
        unique_count_++;
        return true;
    }
    return false;
}

void Triage::classify(const ExecResult& exec,
                      const Script& script,
                      const std::vector<PatternHit>& patterns)
{
    bus_.publish("triage:start", "triage started");

    uint64_t sig = compute_signature(exec, patterns);

    if (exec.status == ExecStatus::CRASH ||
        exec.status == ExecStatus::TIMEOUT ||
        exec.status == ExecStatus::SANDBOX_FAILURE)
    {
        bus_.publish("triage:crash", "crash detected");

        if (is_unique(sig)) {
            bus_.publish("triage:unique", "unique crash");

            CrashEntry entry;
            entry.signature = std::to_string(sig);
            entry.original  = script.code;
            entry.minimized = ""; // minimizer fills later
            entry.patterns  = patterns;
            entry.exec      = exec;
            entry.timestamp = std::time(nullptr);

            store_.save(entry);
        }
    }
    else if (exec.status == ExecStatus::JS_EXCEPTION) {
        bus_.publish("triage:js_exception", exec.stderr_log);
    }
    else {
        bus_.publish("triage:ok", "no crash");
    }

    bus_.publish("triage:end", "triage finished");
}
