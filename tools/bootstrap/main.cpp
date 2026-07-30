#include <cstdlib>
#include <iostream>
#include <unistd.h>

#include "diagnostics.h"

int main() {
    chdir("/mnt/c/Users/freeg/Documents/aniva_crash");

    // ВОТ ЭТА СТРОКА — ФИКС ВСЕЙ ХУЙНИ
    setenv("PATH", "/usr/local/bin:/usr/bin:/bin:/snap/bin:/usr/local/cuda/bin", 1);

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
