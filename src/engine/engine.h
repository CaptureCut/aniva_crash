#pragma once

#include <atomic>
#include <memory>
#include <string>

#include "engine_config.h"

#include "bus/event_bus.h"
#include "telemetry/telemetry.h"
#include "time/heartbeat.h"

#include "persistence/corpus_store.h"
#include "persistence/crash_store.h"

#include "isolation/process_sandbox.h"

#include "gpu/gpu_runtime.h"
#include "gpu/gpu_bias.h"

#include "core/generator/generator.h"
#include "core/executor/executor.h"
#include "core/triage/triage.h"
#include "core/minimizer/minimizer.h"
#include "core/pattern/pattern_analyzer.h"
#include "core/hunt_loop/hunt_loop.h"

class Engine {
public:
    explicit Engine(const EngineConfig& cfg);

    void init();
    void warmup();
    void run();
    void shutdown();

private:
    EngineConfig cfg_;
    std::atomic<bool> stop_flag_{false};

    EventBus bus_;
    Telemetry telemetry_;
    Heartbeat heartbeat_;

    CorpusStore corpus_;
    CrashStore crashes_;

    ProcessSandbox sandbox_;

    GpuRuntime gpu_runtime_;
    GpuBias gpu_bias_;

    std::unique_ptr<Generator> generator_;
    std::unique_ptr<Executor> executor_;
    std::unique_ptr<Triage> triage_;
    std::unique_ptr<Minimizer> minimizer_;
    std::unique_ptr<PatternAnalyzer> analyzer_;
    std::unique_ptr<HuntLoop> hunt_loop_;
};
