#pragma once
#include <string>
#include "core/executor/exec_status.h"

struct ExecResult {
    ExecStatus status = ExecStatus::OK;

    int exit_code = 0;
    int signal = 0;

    std::string crash_sig;
    std::string stdout_log;
    std::string stderr_log;
};
