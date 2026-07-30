#include "runner.h"
#include "event_bus.h"
#include "process_sandbox.h"

Runner::Runner(EventBus& bus, ProcessSandbox& sandbox)
    : bus_(bus), sandbox_(sandbox)
{}

RunResult Runner::run(const Script& script) {
    bus_.emit("run_start", "process sandbox runner started");

    RunResult r = sandbox_.run_script(script.code);

    // stdout
    if (!r.stdout_data.empty()) {
        bus_.emit("run_stdout", r.stdout_data);
    }

    // stderr
    if (!r.stderr_data.empty()) {
        bus_.emit("run_stderr", r.stderr_data);
    }

    // exit code reporting
    if (r.exit_code != 0) {
        bus_.emit("run_error", "non-zero exit code: " + std::to_string(r.exit_code));
    }

    bus_.emit("run_end", "process sandbox runner finished");
    return r;
}
