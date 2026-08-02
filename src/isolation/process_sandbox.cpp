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
    res.crash_sig.clear();
    res.stdout_log.clear();
    res.stderr_log.clear();

    int out_pipe[2]{-1, -1};
    int err_pipe[2]{-1, -1};

    auto safe_close = [](int fd) {
        if (fd >= 0) ::close(fd);
    };

    // -------------------------
    // PIPE SETUP
    // -------------------------
    if (pipe(out_pipe) < 0) {
        res.crash_sig = "PIPE_OUT_FAIL";
        return res;
    }
    if (pipe(err_pipe) < 0) {
        res.crash_sig = "PIPE_ERR_FAIL";
        safe_close(out_pipe[0]);
        safe_close(out_pipe[1]);
        return res;
    }

    // -------------------------
    // FORK
    // -------------------------
    pid_t pid = fork();
    if (pid < 0) {
        res.crash_sig = "FORK_FAIL";
        safe_close(out_pipe[0]); safe_close(out_pipe[1]);
        safe_close(err_pipe[0]); safe_close(err_pipe[1]);
        return res;
    }

    if (pid == 0) {
        // CHILD
        dup2(out_pipe[1], STDOUT_FILENO);
        dup2(err_pipe[1], STDERR_FILENO);

        safe_close(out_pipe[0]);
        safe_close(err_pipe[0]);
        safe_close(out_pipe[1]);
        safe_close(err_pipe[1]);

        const char* argv[] = {
            engine_path_.c_str(),
            "-e",
            script_code.c_str(),
            "--allow-natives-syntax",
            "--no-wasm-trap-handler",
            nullptr
        };

        execv(engine_path_.c_str(), const_cast<char* const*>(argv));
        _exit(1);
    }

    // PARENT
    safe_close(out_pipe[1]);
    safe_close(err_pipe[1]);

    struct pollfd fds[2];
    fds[0].fd = out_pipe[0];
    fds[0].events = POLLIN;
    fds[1].fd = err_pipe[0];
    fds[1].events = POLLIN;

    char buf[4096];
    bool timed_out = false;

    // -------------------------
    // POLL LOOP
    // -------------------------
    while (true) {
        int ret = poll(fds, 2, timeout_ms_);

        if (ret == 0) {
            timed_out = true;
            kill(pid, SIGKILL);
            break;
        }

        if (ret < 0) {
            if (errno == EINTR) continue;
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

        int status = 0;
        pid_t w = waitpid(pid, &status, WNOHANG);
        if (w == pid) {
            if (WIFEXITED(status)) {
                res.exit_code = WEXITSTATUS(status);
                // здесь sandbox НЕ решает, JS_EXCEPTION это или нет
                // он просто говорит: процесс завершился нормально
                res.status = ExecStatus::OK;
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

    safe_close(out_pipe[0]);
    safe_close(err_pipe[0]);

    if (timed_out) {
        res.status = ExecStatus::TIMEOUT;
        res.crash_sig = "TIMEOUT";
        res.signal = SIGKILL; // чтобы triage/executor могли понять, что это watchdog
    }

    return res;
}
