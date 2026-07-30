#pragma once
#include <string>
#include <atomic>
#include "bus/event_bus.h"
#include "executor_result.h"
#include "isolation/process_sandbox.h"
#include "time/timeout.h"
#include "memory/bounded_buffer.h"

class Executor {
public:
    Executor(EventBus& bus,
             ProcessSandbox& sandbox,
             int timeout_ms);

    ExecResult execute(const std::string& script_code);

private:
    ExecResult run_in_sandbox(const std::string& script_code);

    ExecStatus classify_exit(int exit_code, int signal);

    EventBus& bus_;
    ProcessSandbox& sandbox_;
    int timeout_ms_;
};
