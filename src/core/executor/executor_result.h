#pragma once
#include <string>
#include <chrono>
#include "core/executor/exec_status.h"   // ← важно

struct ExecResult {
    ExecStatus status = ExecStatus::OK;

    int exit_code = 0;
    int signal = 0;

    std::string crash_sig;
    std::string stdout_log;
    std::string stderr_log;

    std::chrono::microseconds exec_time{0};
    size_t script_size = 0;

    bool killed_by_watchdog = false;
    uint64_t signature = 0;
};
