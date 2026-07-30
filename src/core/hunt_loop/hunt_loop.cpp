#include "core/hunt_loop/hunt_loop.h"
#include <chrono>
#include <ctime>

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

void HuntLoop::run(std::atomic<bool>& stop_flag) {
    bus_.publish("hunt:start", "hunt loop started");

    std::size_t iteration = 0;

    while (!stop_flag.load(std::memory_order_relaxed)) {
        ++iteration;

        heartbeat_.tick(iteration);
        telemetry_.inc("iterations");
        bus_.publish("hunt:iter", "iteration " + std::to_string(iteration));

        auto t0 = std::chrono::high_resolution_clock::now();

        // 1) generate JS
        Script script = generator_.generate();
        telemetry_.inc("generated");

        // 2) analyze patterns
        std::vector<PatternHit> patterns = analyzer_.analyze(script.code);
        telemetry_.inc("patterns_found", patterns.size());

        // feedback: update generator bias
        generator_.update_bias(patterns);

        // 3) execute with sandbox
        ExecResult exec = executor_.execute(script);
        telemetry_.inc("executed");

        // 4) triage with full context
        triage_.classify(exec, script, patterns);

        // 5) save interesting OK cases into corpus
        bool interesting = !patterns.empty(); // временный критерий
        if (exec.status == ExecStatus::OK && interesting) {
            corpus_.save(script.code);
            telemetry_.inc("corpus_saved");
        }

        // 6) crash / timeout / sandbox failure / JS exception → minimize + save
        if (exec.status == ExecStatus::CRASH ||
            exec.status == ExecStatus::TIMEOUT ||
            exec.status == ExecStatus::SANDBOX_FAILURE ||
            exec.status == ExecStatus::JS_EXCEPTION)
        {
            telemetry_.inc("crashes");

            // minimize JS code
            std::string minimized = minimizer_.minimize(script.code);

            // build crash entry
            CrashEntry entry;
            entry.signature = std::hash<std::string>{}(script.code);
            entry.original   = script.code;
            entry.minimized  = minimized;
            entry.patterns   = patterns;
            entry.exec       = exec;
            entry.timestamp  = std::time(nullptr);

            crashes_.save(entry);

            bus_.publish("hunt:poc", minimized);
        }

        auto t1 = std::chrono::high_resolution_clock::now();
        telemetry_.add_time(
            "iteration_us",
            std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0)
        );
    }

    bus_.publish("hunt:end", "hunt loop finished");
}
