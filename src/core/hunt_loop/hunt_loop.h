#pragma once

#include <atomic>

#include "bus/event_bus.h"
#include "core/generator/generator.h"
#include "core/executor/executor.h"
#include "core/triage/triage.h"
#include "core/minimizer/minimizer.h"
#include "core/pattern/pattern_analyzer.h"

#include "persistence/corpus_store.h"
#include "persistence/crash_store.h"

#include "telemetry/telemetry.h"
#include "time/heartbeat.h"

class HuntLoop {
public:
    HuntLoop(EventBus& bus,
             Generator& generator,
             Executor& executor,
             Triage& triage,
             Minimizer& minimizer,
             PatternAnalyzer& analyzer,
             CorpusStore& corpus,
             CrashStore& crashes,
             Telemetry& telemetry,
             Heartbeat& heartbeat);

    void run(std::atomic<bool>& stop_flag);

private:
    EventBus& bus_;
    Generator& generator_;
    Executor& executor_;
    Triage& triage_;
    Minimizer& minimizer_;
    PatternAnalyzer& analyzer_;
    CorpusStore& corpus_;
    CrashStore& crashes_;
    Telemetry& telemetry_;
    Heartbeat& heartbeat_;
};
