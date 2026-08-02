#pragma once

#include <string>
#include "core/executor/executor_result.h"

class ProcessSandbox {
public:
    // конструктор: путь к d8 + таймаут
    ProcessSandbox(const std::string& engine_path, int timeout_ms);

    // подготовка окружения (опционально)
    void warmup();

    // корректное завершение (опционально)
    void shutdown();

    // запуск JS-кода в песочнице
    ExecResult run_script(const std::string& script_code);

private:
    // путь к бинарю d8
    std::string engine_path_;

    // таймаут исполнения JS
    int timeout_ms_;

    // проверка наличия движка
    void ensure_engine();
};
