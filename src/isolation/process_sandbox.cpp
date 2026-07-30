#include "isolation/process_sandbox.h"

#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <poll.h>
#include <fcntl.h>
#include <stdexcept>
#include <cstring>
#include <filesystem>

ProcessSandbox::ProcessSandbox(const std::string& engine_path, int timeout_ms)
    : engine_path_(engine_path)
    , timeout_ms_(timeout_ms)
{
    ensure_engine();
}

void ProcessSandbox::ensure_engine() {
    if (!std::filesystem::exists(engine_path_)) {
        throw std::runtime_error("ProcessSandbox: engine not found: " + engine_path_);
    }
}

void ProcessSandbox::warmup() {
    // можно добавить preload, mmap, подготовку окружения
}

void ProcessSandbox::shutdown() {
    // можно добавить очистку ресурсов
}

ExecResult ProcessSandbox::run_script(const std::string& script_code) {
    ExecResult res{};
    res.status    = ExecStatus::SANDBOX_FAILURE;
    res.exit_code = -1;
    res.signal    = 0;

    int in_pipe[2];
    int out_pipe[2];
    int err_pipe[2];

    if (pipe(in_pipe) < 0) {
        res.crash_sig = "PIPE_IN_FAIL";
        return res;
    }
    if (pipe(out_pipe) < 0) {
        res.crash_sig = "PIPE_OUT_FAIL";
        close(in_pipe[0]);
        close(in_pipe[1]);
        return res;
    }
    if (pipe(err_pipe) < 0) {
        res.crash_sig = "PIPE_ERR_FAIL";
        close(in_pipe[0]);
        close(in_pipe[1]);
        close(out_pipe[0]);
        close(out_pipe[1]);
        return res;
    }

    pid_t pid = fork();
    if (pid < 0) {
        res.crash_sig = "FORK_FAIL";
        close(in_pipe[0]);  close(in_pipe[1]);
        close(out_pipe[0]); close(out_pipe[1]);
        close(err_pipe[0]); close(err_pipe[1]);
        return res;
    }

    if (pid == 0) {
        // CHILD
        dup2(in_pipe[0], STDIN_FILENO);
        dup2(out_pipe[1], STDOUT_FILENO);
        dup2(err_pipe[1], STDERR_FILENO);

        close(in_pipe[1]);
        close(out_pipe[0]);
        close(err_pipe[0]);

        execl(engine_path_.c_str(), "d8",
              "--allow-natives-syntax",
              "--no-wasm-trap-handler",
              nullptr);

        _exit(1);
    }

    // PARENT
    close(in_pipe[0]);
    close(out_pipe[1]);
    close(err_pipe[1]);

    ssize_t written = write(in_pipe[1], script_code.data(), script_code.size());
    (void)written; // можно логировать при желании
    close(in_pipe[1]);

    struct pollfd fds[2];
    fds[0].fd = out_pipe[0];
    fds[0].events = POLLIN;
    fds[1].fd = err_pipe[0];
    fds[1].events = POLLIN;

    char buf[4096];
    bool timed_out = false;

    while (true) {
        int ret = poll(fds, 2, timeout_ms_);

        if (ret == 0) {
            timed_out = true;
            kill(pid, SIGKILL);
            break;
        }

        if (ret < 0) {
            res.crash_sig = "POLL_FAIL";
            break;
        }

        if (fds[0].revents & POLLIN) {
            ssize_t n = read(out_pipe[0], buf, sizeof(buf));
            if (n > 0) res.stdout_log.append(buf, n);
        }

        if (fds[1].revents & POLLIN) {
            ssize_t n = read(err_pipe[0], buf, sizeof(buf));
            if (n > 0) res.stderr_log.append(buf, n);
        }

        int status;
        pid_t w = waitpid(pid, &status, WNOHANG);
        if (w == pid) {
            if (WIFEXITED(status)) {
                res.exit_code = WEXITSTATUS(status);
                if (res.exit_code == 0) {
                    res.status = ExecStatus::OK;
                } else {
                    res.status = ExecStatus::JS_EXCEPTION;
                    res.crash_sig = "JS_EXIT_" + std::to_string(res.exit_code);
                }
            } else if (WIFSIGNALED(status)) {
                res.signal = WTERMSIG(status);
                res.status = ExecStatus::CRASH;

                switch (res.signal) {
                    case SIGSEGV: res.crash_sig = "SIGSEGV"; break;
                    case SIGABRT: res.crash_sig = "SIGABRT"; break;
                    case SIGILL:  res.crash_sig = "SIGILL";  break;
                    case SIGBUS:  res.crash_sig = "SIGBUS";  break;
                    default:      res.crash_sig = "SIGNAL_" + std::to_string(res.signal);
                }
            }
            break;
        }
    }

    close(out_pipe[0]);
    close(err_pipe[0]);

    if (timed_out) {
        res.status = ExecStatus::TIMEOUT;
        res.crash_sig = "TIMEOUT";
    }

    return res;
}
