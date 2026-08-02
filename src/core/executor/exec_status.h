#pragma once

enum class ExecStatus {
    OK,             // успешное выполнение
    TIMEOUT,        // превышен таймаут
    CRASH,          // процесс упал (SIGSEGV, SIGABRT и т.п.)
    JS_EXCEPTION,   // JS-исключение (exit_code != 0, signal == 0)
    SANDBOX_FAILURE,// ошибка sandbox (fork/exec/pipe/IPC)
    INTERESTING     // интересный кейс (не crash, но стоит сохранить)
};
