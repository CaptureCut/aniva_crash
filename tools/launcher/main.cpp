#include <iostream>
#include <cstdlib>

int main() {
    std::cout << "[launcher] checking for rebuild\n";

    int rebuild_code = system("./tools/self_rebuild/self_rebuild");

    if (rebuild_code != 0) {
        std::cerr << "[launcher] rebuild failed, aborting\n";
        return 1;
    }

    std::cout << "[launcher] rebuild ok → starting engine\n";

    int run_code = system("./build/aniva_crash");

    if (run_code != 0) {
        std::cerr << "[launcher] engine exited with error code " << run_code << "\n";
    }

    return run_code;
}
