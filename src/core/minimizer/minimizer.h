#pragma once

#include <string>
#include <vector>

#include "bus/event_bus.h"
#include "core/executor/executor.h"
#include "core/executor/executor_result.h"
#include "core/generator/script.h"
#include "persistence/crash_store.h"

class Minimizer {
public:
    Minimizer(EventBus& bus,
              Executor& executor,
              CrashStore& store);

    // минимизация принимает только JS-код
    std::string minimize(const std::string& code);

private:
    bool still_crashes(const std::string& js);

    EventBus& bus_;
    Executor& executor_;
    CrashStore& store_;
};
