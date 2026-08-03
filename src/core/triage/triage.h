#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <unordered_set>

#include "bus/event_bus.h"
#include "core/executor/executor_result.h"
#include "core/pattern/pattern_analyzer.h"
#include "persistence/crash_store.h"
#include "core/generator/script.h"
#include "core/triage/triage_result.h"

// forward declaration
class CorpusStore;

class Triage {
public:
    Triage(EventBus& bus, CrashStore& store, CorpusStore& corpus);

    void classify(const ExecResult& exec,
                  const Script& script,
                  const std::vector<PatternHit>& patterns);

    // HuntLoop::restart()
    void reset();

private:
    // ------------------------------------------------------------
    // Crash-key (deduplication)
    // ------------------------------------------------------------
    uint64_t compute_crash_key(const ExecResult& exec);

    // legacy signature (optional)
    uint64_t compute_signature(const ExecResult& exec,
                               const std::vector<PatternHit>& patterns);

    bool is_unique(uint64_t sig);

    // ------------------------------------------------------------
    // Crash classification helpers
    // ------------------------------------------------------------
    bool is_real_crash(const ExecResult& exec);
    bool is_timeout(const ExecResult& exec);
    bool is_js_exception(const ExecResult& exec);
    bool is_sandbox_error(const ExecResult& exec);

    // severity 0–5
    int severity(const ExecResult& exec);

    // ------------------------------------------------------------
    // Logging
    // ------------------------------------------------------------
    void log_alert(const std::string& msg);

    // ------------------------------------------------------------
    // Stacktrace saving
    // ------------------------------------------------------------
    void save_stacktrace(const ExecResult& exec);

private:
    EventBus& bus_;
    CrashStore& store_;
    CorpusStore& corpus_;

    // old unique detection
    uint64_t last_sig_ = 0;
    uint64_t unique_count_ = 0;

    // new deduplication
    std::unordered_set<uint64_t> seen_keys_;
};
