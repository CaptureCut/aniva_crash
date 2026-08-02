#include "core/hunt_loop/hunt_loop.h"

#include <atomic>
#include <chrono>
#include <csignal>
#include <fstream>
#include <iostream>
#include <filesystem>   // <— нужно для ротации

using SteadyClock = std::chrono::steady_clock;

// ------------------------------------------------------------
// Auto‑rotation for stats.log
// ------------------------------------------------------------
static void rotate_stats_log(std::size_t max_size_bytes = 2'000'000)
{
    const std::filesystem::path LOG = "logs/stats.log";

    // если файла нет — нечего ротировать
    if (!std::filesystem::exists(LOG))
        return;

    // если размер меньше лимита — всё ок
    if (std::filesystem::file_size(LOG) < max_size_bytes)
        return;

    // создаём имя для архива
    const auto ts = std::time(nullptr);
    auto rotated = LOG.parent_path() / ("stats_" + std::to_string(ts) + ".log");

    // переименовываем старый лог
    std::filesystem::rename(LOG, rotated);

    // создаём новый пустой stats.log
    std::ofstream(LOG).close();
}

// ------------------------------------------------------------
// HuntLoop
// ------------------------------------------------------------
HuntLoop::HuntLoop(EventBus& bus,
                   Generator& generator,
                   Executor& executor,
                   Triage& triage,
                   Minimizer& minimizer,
                   PatternAnalyzer& analyzer,
                   CorpusStore& corpus,
                   CrashStore& crashes,
                   Telemetry& telemetry,
                   Heartbeat& heartbeat)
    : bus_(bus)
    , generator_(generator)
    , executor_(executor)
    , triage_(triage)
    , minimizer_(minimizer)
    , analyzer_(analyzer)
    , corpus_(corpus)
    , crashes_(crashes)
    , telemetry_(telemetry)
    , heartbeat_(heartbeat)
{}

void HuntLoop::restart()
{
    bus_.publish("watchdog:restart", "Restarting hunt loop.");

    generator_.reset();
    analyzer_.reset();
    executor_.reset();
    triage_.reset();

    idle_counter_ = 0;
}

// ------------------------------------------------------------
// Write stats with auto‑rotation
// ------------------------------------------------------------
void HuntLoop::write_stats(std::size_t iteration)
{
    rotate_stats_log();   // <— добавлено

    std::ofstream out("logs/stats.log", std::ios::app);
    if (!out) return;

    out << "iteration=" << iteration
        << " generated=" << telemetry_.get("generated")
        << " executed=" << telemetry_.get("executed")
        << " crashes=" << telemetry_.get("crashes")
        << " corpus_saved=" << telemetry_.get("corpus_saved")
        << " avg_iter_us=" << telemetry_.avg_time("iteration_us")
        << " max_iter_us=" << telemetry_.max_time("iteration_us")
        << "\n";

    bus_.publish("stats:update", "stats written");
}

// ------------------------------------------------------------
// Main hunt loop
// ------------------------------------------------------------
void HuntLoop::run(std::atomic<bool>& stop_flag)
{
    bus_.publish("hunt:start", "hunt loop started");

    std::size_t iteration = 0;

    while (!stop_flag.load(std::memory_order_relaxed)) {

        ++iteration;
        heartbeat_.tick(iteration);
        telemetry_.inc("iterations");
        bus_.publish("hunt:iter", "iteration " + std::to_string(iteration));

        auto t0 = SteadyClock::now();

        // 1) Generate JS
        Script script = generator_.generate();
        telemetry_.inc("generated");

        // 2) Analyze patterns
        std::vector<PatternHit> patterns = analyzer_.analyze(script.code);
        telemetry_.inc("patterns_found", patterns.size());

        generator_.update_bias(patterns);

        // 3) Execute
        ExecResult exec = executor_.execute(script);
        telemetry_.inc("executed");

        // 4) Triage
        triage_.classify(exec, script, patterns);

        // 5) Crash → minimize PoC
        if (exec.status == ExecStatus::CRASH) {
            telemetry_.inc("crashes");

            std::string minimized = minimizer_.minimize(script.code);
            bus_.publish("hunt:poc", minimized);
        }

        // Watchdog: slow iteration
        auto t1 = SteadyClock::now();
        auto us = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
        telemetry_.add_time("iteration_us", std::chrono::microseconds(us));

        if (us > 500000) {
            bus_.publish("watchdog:slow_iteration",
                         "Iteration exceeded 500ms, restarting hunt loop.");
            restart();
            continue;
        }

        // Watchdog: executor timeout
        if (exec.status == ExecStatus::TIMEOUT ||
            exec.signal == SIGKILL)
        {
            bus_.publish("watchdog:executor_timeout",
                         "Executor timeout detected, restarting hunt loop.");
            restart();
            continue;
        }

        // Watchdog: idle (no interesting cases)
        if (exec.status != ExecStatus::INTERESTING)
            idle_counter_++;
        else
            idle_counter_ = 0;

        if (idle_counter_ > 5000) {
            bus_.publish("watchdog:idle",
                         "No interesting cases for too long, restarting hunt loop.");
            restart();
            continue;
        }

        // Auto statistics
        if (iteration % 1000 == 0) {
            write_stats(iteration);
        }
    }

    bus_.publish("hunt:end", "hunt loop finished");
}
