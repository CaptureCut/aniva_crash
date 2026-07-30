#pragma once

#include <string>
#include <vector>
#include <ctime>

#include "core/pattern/pattern_analyzer.h"
#include "core/executor/executor_result.h"

struct CrashEntry {
    std::string original;          // исходный JS-код
    std::string minimized;         // минимизированный PoC
    std::vector<PatternHit> patterns; // паттерны
    ExecResult exec;               // результат выполнения
    std::time_t timestamp;         // время
    std::string signature;         // сигнатура (можно пустую)
};
