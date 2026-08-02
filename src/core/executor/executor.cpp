#include "core/executor/executor.h"
#include "bus/event_bus.h"
#include <csignal>

Executor::Executor(EventBus& bus, ProcessSandbox& sandbox)
    : bus_(bus)
    , sandbox_(sandbox)
{}

ExecResult Executor::execute(const Script& script) {
    bus_.publish("exec:start", "executor started");

    // ------------------------------------------------------------
    //  RUN SCRIPT IN SANDBOX
    // ------------------------------------------------------------
    ExecResult res = sandbox_.run_script(script.code);

    // ------------------------------------------------------------
    //  RAW LOGGING
    // ------------------------------------------------------------
    bus_.publish("exec:raw",
        "exit="   + std::to_string(res.exit_code) +
        " signal=" + std::to_string(res.signal) +
        " status=" + std::to_string(static_cast<int>(res.status)));

    // ------------------------------------------------------------
    //  CLASSIFICATION LAYER
    // ------------------------------------------------------------

    // JS exception: non-zero exit code, no signal, status OK
    if (res.status == ExecStatus::OK &&
        res.signal == 0 &&
        res.exit_code != 0)
    {
        res.status = ExecStatus::JS_EXCEPTION;
    }

    // SIGKILL → watchdog timeout
    if (res.signal == SIGKILL &&
        res.status != ExecStatus::TIMEOUT)
    {
        res.status = ExecStatus::TIMEOUT;
    }

    // SIGTERM → external kill, not crash
    if (res.signal == SIGTERM &&
        res.status == ExecStatus::CRASH)
    {
        res.status = ExecStatus::SANDBOX_FAILURE;
    }

    // ------------------------------------------------------------
    //  LOG CLASSIFIED RESULT
    // ------------------------------------------------------------
    switch (res.status) {
        case ExecStatus::OK:
            bus_.publish("exec:ok", "script executed ok");
            break;

        case ExecStatus::JS_EXCEPTION:
            bus_.publish("exec:js_exception", res.stderr_log);
            break;

        case ExecStatus::CRASH:
            bus_.publish("exec:crash", "native crash: " + res.crash_sig);
            break;

        case ExecStatus::TIMEOUT:
            bus_.publish("exec:timeout", "execution timed out");
            break;

        case ExecStatus::SANDBOX_FAILURE:
        default:
            bus_.publish("exec:error", "sandbox failure");
            break;
    }

    bus_.publish("exec:end", "executor finished");
    return res;
}

void Executor::reset() {
    // пусто — но нужно для HuntLoop::restart()
}
