#include "core/executor/executor.h"

Executor::Executor(EventBus& bus,
                   ProcessSandbox& sandbox)
    : bus_(bus)
    , sandbox_(sandbox)
{}

ExecResult Executor::execute(const Script& script) {
    bus_.publish("exec:start", "executor started");

    // sandbox already handles timeout internally
    ExecResult res = sandbox_.run_script(script.code);

    // classify result
    res.status = classify_exit(res);

    switch (res.status) {
    case ExecStatus::OK:
        bus_.publish("exec:ok", "script executed ok");
        break;

    case ExecStatus::TIMEOUT:
        bus_.publish("exec:timeout", "script timeout");
        break;

    case ExecStatus::CRASH:
        bus_.publish("exec:crash", "script crashed: " + res.crash_sig);
        break;

    case ExecStatus::SANDBOX_FAILURE:
        bus_.publish("exec:error", "sandbox failure");
        break;

    case ExecStatus::JS_EXCEPTION:
        bus_.publish("exec:js_exception", res.stderr_log);
        break;
    }

    bus_.publish("exec:end", "executor finished");
    return res;
}

ExecStatus Executor::classify_exit(const ExecResult& res) {
    // sandbox already sets CRASH / TIMEOUT / SANDBOX_FAILURE
    if (res.status == ExecStatus::CRASH ||
        res.status == ExecStatus::TIMEOUT ||
        res.status == ExecStatus::SANDBOX_FAILURE)
    {
        return res.status;
    }

    // JS exception = non-zero exit code without signal
    if (res.signal == 0 && res.exit_code != 0) {
        return ExecStatus::JS_EXCEPTION;
    }

    return ExecStatus::OK;
}
