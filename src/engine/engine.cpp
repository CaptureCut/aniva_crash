#include "engine/engine.h"

Engine::Engine(const EngineConfig& cfg)
    : cfg_(cfg)
    , bus_()
    , telemetry_()
    , heartbeat_(telemetry_, cfg.heartbeat_ms)
    , corpus_(cfg.corpus_dir)
    , crashes_(cfg.crash_dir)
    , sandbox_(cfg.engine_path, cfg.timeout_ms)
    , gpu_runtime_()
    , gpu_bias_(bus_, gpu_runtime_)
{}

void Engine::init() {
    // порядок создания строго соответствует новым конструкторам
    generator_ = std::make_unique<Generator>(
        bus_,
        &gpu_bias_,
        &corpus_
    );

    executor_ = std::make_unique<Executor>(
        bus_,
        sandbox_
    );

    triage_ = std::make_unique<Triage>(
        bus_,
        crashes_
    );

    minimizer_ = std::make_unique<Minimizer>(
        bus_,
        *executor_,
        crashes_
    );

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

    heartbeat_.start();
}

void Engine::warmup() {
    bus_.publish("engine:warmup", "warming up engine");
    sandbox_.warmup();
}

void Engine::run() {
    bus_.publish("engine:start", "engine started");
    stop_flag_.store(false);
    hunt_loop_->run(stop_flag_);
}

void Engine::shutdown() {
    stop_flag_.store(true);
    heartbeat_.stop();
    sandbox_.shutdown();
    bus_.publish("engine:shutdown", "engine stopped");
}
