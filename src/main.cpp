#include <filesystem>
#include <cstdlib>
#include <vector>
#include <string>
#include <atomic>

#include "bus/event_bus.h"
#include "diagnostics/live_diagnostics.h"

#include "gpu/gpu_runtime.h"
#include "gpu/gpu_bias.h"

#include "isolation/process_sandbox.h"

#include "core/generator/generator.h"
#include "core/executor/executor.h"
#include "core/triage/triage.h"
#include "core/minimizer/minimizer.h"
#include "core/pattern/pattern_analyzer.h"

#include "persistence/corpus_store.h"
#include "persistence/crash_store.h"

#include "telemetry/telemetry.h"
#include "time/heartbeat.h"

#include "core/hunt_loop/hunt_loop.h"

namespace fs = std::filesystem;

int main() {
    EventBus bus;
    LiveDiagnostics diag(bus);

    // === Directories ===
    std::string corpus_dir = "data/corpus";
    std::string crash_dir  = "data/crashes";

    // === Stores ===
    CorpusStore corpus(corpus_dir, bus);
    CrashStore crashes(crash_dir);

    // === GPU Runtime (no args) ===
    GpuRuntime gpu_runtime;

    // === GPU Bias ===
    GpuBias gpu_bias(bus, gpu_runtime);

    // === Sandbox ===
    ProcessSandbox sandbox(
        "/home/null0e/v8_clean/v8/out/x64.release/d8",
        5000
    );

    // === Components ===
    Generator generator(bus, &gpu_bias, &corpus);
    Executor executor(bus, sandbox);
    Triage triage(bus, crashes, corpus);
    Minimizer minimizer(bus, executor, crashes);
    PatternAnalyzer pattern;
    Telemetry telemetry;
    Heartbeat heartbeat(telemetry, 1000);

    // === HuntLoop ===
    HuntLoop loop(
        bus,
        generator,
        executor,
        triage,
        minimizer,
        pattern,
        corpus,
        crashes,
        telemetry,
        heartbeat
    );

    std::atomic<bool> stop_flag(false);

    bus.emit("start", "starting hunt loop");

    loop.run(stop_flag);

    return 0;
}
