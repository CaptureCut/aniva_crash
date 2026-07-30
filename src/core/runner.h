#pragma once
#include <string>
#include "event_bus.h"
#include "process_sandbox.h"
#include "script.h"
#include "run_result.h"

class Runner {
public:
    Runner(EventBus& bus, ProcessSandbox& sandbox);

    RunResult run(const Script& script);

private:
    EventBus& bus_;
    ProcessSandbox& sandbox_;
};
