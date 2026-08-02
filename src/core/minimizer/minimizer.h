#pragma once

#include <string>
#include <vector>
#include <functional>

#include "bus/event_bus.h"
#include "core/executor/executor.h"
#include "core/executor/executor_result.h"
#include "core/generator/script.h"
#include "persistence/crash_store.h"

// forward declarations
class AstMin;
class Delta;

class Minimizer {
public:
    Minimizer(EventBus& bus,
              Executor& executor,
              CrashStore& store);

    // Минимизирует JS-код, сохраняя крэш
    std::string minimize(const std::string& code);

private:
    // Проверяет, что код всё ещё вызывает крэш
    bool still_crashes(const std::string& js);

    EventBus& bus_;
    Executor& executor_;
    CrashStore& store_;
};
