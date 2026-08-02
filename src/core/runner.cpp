#include "runner.h"
#include "event_bus.h"
#include "process_sandbox.h"

Runner::Runner(EventBus& bus, ProcessSandbox& sandbox)
    : bus_(bus)
    , sandbox_(sandbox)
{}

RunResult Runner::run(const Script& script) {
    bus_.emit("run_start", "runner started");

    RunResult r = sandbox_.run_script(script.code);

    // ------------------------------------------------------------
    // RAW LOG
    // ------------------------------------------------------------
    bus_.emit("run_raw",
        "exit=" + std::to_string(r.exit_code) +
        " signal=" + std::to_string(r.signal) +
        " status=" + std::to_string(static_cast<int>(r.status)));

    // ------------------------------------------------------------
    // STDOUT / STDERR
    // ------------------------------------------------------------
    if (!r.stdout_data.empty())
        bus_.emit("run_stdout", r.stdout_data);

    if (!r.stderr_data.empty())
        bus_.emit("run_stderr", r.stderr_data);

    // ------------------------------------------------------------
    // STATUS LOGGING
    // ------------------------------------------------------------
    switch (r.status) {
        case ExecStatus::OK:
            bus_.emit("run_ok", "script executed ok");
            break;

        case ExecStatus::JS_EXCEPTION:
            bus_.emit("run_js_exception", r.stderr_data);
            break;

        case ExecStatus::CRASH:
            bus_.emit("run_crash", "native crash: " + r.crash_sig);
            break;

        case ExecStatus::TIMEOUT:
            bus_.emit("run_timeout", "execution timed out");
            break;

        case ExecStatus::SANDBOX_FAILURE:
        default:
            bus_.emit("run_error", "sandbox failure");
            break;
    }

    bus_.emit("run_end", "runner finished");
    return r;
}
