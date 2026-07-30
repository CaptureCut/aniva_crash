#include <filesystem>
#include <cstdlib>
#include <vector>
#include <string>
#include <iostream>

namespace fs = std::filesystem;

// собираем список всех исходников
std::vector<fs::path> collect_sources() {
    std::vector<fs::path> files;

    for (auto& p : fs::recursive_directory_iterator("src")) {
        if (p.is_regular_file()) {
            files.push_back(p.path());
        }
    }

    return files;
}

bool sources_changed(const fs::path& exe_path) {
    if (!fs::exists(exe_path)) {
        // бинаря нет — считаем, что нужно собрать
        return true;
    }

    auto exe_time = fs::last_write_time(exe_path);

    for (auto& file : collect_sources()) {
        if (fs::last_write_time(file) > exe_time) {
            return true;
        }
    }

    return false;
}

bool rebuild() {
#ifdef _WIN32
    int code = system("powershell -File build/build.ps1");
#else
    int code = system("bash build/build.sh");
#endif
    return code == 0;
}

int main() {
#ifdef _WIN32
    fs::path exe_path = "build\\aniva_crash.exe";
#else
    fs::path exe_path = "build/aniva_crash";
#endif

    if (!sources_changed(exe_path)) {
        std::cout << "[self_rebuild] no changes, nothing to do\n";
        return 0;
    }

    std::cout << "[self_rebuild] sources changed → rebuilding\n";

    if (!rebuild()) {
        std::cerr << "[self_rebuild] rebuild failed\n";
        return 1;
    }

    std::cout << "[self_rebuild] rebuild ok\n";
    return 0;
}
