#pragma once

#include <string>
#include "bus/event_bus.h"
#include "core/executor/executor_result.h"
#include "core/generator/script.h"
#include "isolation/process_sandbox.h"

class Executor {
public:
    Executor(EventBus& bus,
             ProcessSandbox& sandbox);

    ExecResult execute(const Script& script);

private:
    ExecStatus classify_exit(const ExecResult& res);

    EventBus& bus_;
    ProcessSandbox& sandbox_;
};
