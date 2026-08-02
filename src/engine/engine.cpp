#include "engine/engine.h"

Engine::Engine(const EngineConfig& cfg)
    : cfg_(cfg)
    , bus_()
    , telemetry_()
    , heartbeat_(telemetry_, cfg.heartbeat_ms)
    , corpus_(cfg.corpus_dir, bus_)          // CorpusStore(dir, bus)
    , crashes_(cfg.crash_dir)                // CrashStore(dir) — FIXED
    , sandbox_(cfg.engine_path, cfg.timeout_ms)
    , gpu_runtime_()
    , gpu_bias_(bus_, gpu_runtime_)
{
}

void Engine::init() {
    bus_.publish("engine:init", "initializing engine");

    generator_ = std::make_unique<Generator>(
        bus_, &gpu_bias_, &corpus_);

    executor_ = std::make_unique<Executor>(
        bus_, sandbox_);

    triage_ = std::make_unique<Triage>(
        bus_, crashes_, corpus_);            // Triage(bus, crashes, corpus)

    minimizer_ = std::make_unique<Minimizer>(
        bus_, *executor_, crashes_);

    analyzer_ = std::make_unique<PatternAnalyzer>();

    hunt_loop_ = std::make_unique<HuntLoop>(
        bus_,
        *generator_,
        *executor_,
        *triage_,
        *minimizer_,
        *analyzer_,
        corpus_,
        crashes_,
        telemetry_,
        heartbeat_
    );

    bus_.publish("engine:init_done", "engine initialization complete");
}

void Engine::warmup() {
    bus_.publish("engine:warmup", "warming up engine");

    gpu_bias_.warmup();
    sandbox_.warmup();

    bus_.publish("engine:warmup_done", "warmup complete");
}

void Engine::run() {
    bus_.publish("engine:run", "engine started");

    stop_flag_.store(false);
    hunt_loop_->run(stop_flag_);

    bus_.publish("engine:run_done", "engine stopped");
}

void Engine::shutdown() {
    bus_.publish("engine:shutdown", "engine shutting down");

    stop_flag_.store(true);

    sandbox_.shutdown();
    gpu_bias_.shutdown();

    bus_.publish("engine:shutdown_done", "engine shutdown complete");
}
