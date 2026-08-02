#include <cstdlib>
#include <iostream>
#include <unistd.h>

#include "diagnostics.h"

int main() {
    // Переходим в корень проекта
    chdir("/mnt/c/Users/freeg/Documents/aniva_crash");

    // Восстанавливаем нормальный PATH
    const char* default_path =
        "/usr/local/bin:"
        "/usr/bin:"
        "/bin:"
        "/snap/bin:"
        "/usr/local/cuda/bin";

    setenv("PATH", default_path, 1);

    std::cout << "[BOOTSTRAP] Running diagnostics...\n";
    diagnostics::run();

    std::cout << "[BOOTSTRAP] Running build...\n";

    int ret = std::system("./build.sh");
    if (ret != 0) {
        std::cerr << "[BOOTSTRAP] Build failed. Fix errors and rerun.\n";
        return ret;
    }

    std::cout << "[BOOTSTRAP] Build OK. Launching engine...\n";
    return std::system("./build/aniva_crash");
}
