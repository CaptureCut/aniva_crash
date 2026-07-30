#pragma once

#include <string>
#include <vector>
#include <cstdint>

#include "bus/event_bus.h"
#include "core/executor/executor_result.h"
#include "core/pattern/pattern_analyzer.h"
#include "persistence/crash_store.h"
#include "core/generator/script.h"

class Triage {
public:
    Triage(EventBus& bus, CrashStore& store);

    void classify(const ExecResult& exec,
                  const Script& script,
                  const std::vector<PatternHit>& patterns);

private:
    uint64_t compute_signature(const ExecResult& exec,
                               const std::vector<PatternHit>& patterns);

    bool is_unique(uint64_t sig);

    EventBus& bus_;
    CrashStore& store_;

    uint64_t last_sig_ = 0;
    uint64_t unique_count_ = 0;
};
