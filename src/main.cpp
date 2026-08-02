#include <filesystem>
#include <cstdlib>
#include <vector>
#include <string>
#include "bus/event_bus.h"
#include "diagnostics/live_diagnostics.h"
#include "core/hunt_loop.h"

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

bool sources_changed() {
    auto exe_time = fs::last_write_time("build/aniva_crash");

    for (auto& file : collect_sources()) {
        if (fs::last_write_time(file) > exe_time) {
            return true;
        }
    }

    return false;
}

bool rebuild(EventBus& bus) {
#ifdef _WIN32
    int code = system("powershell -File build/build.ps1");
#else
    int code = system("bash build/build.sh");
#endif

    if (code == 0) {
        bus.emit("rebuild_ok", "rebuild completed");
        return true;
    } else {
        bus.emit("rebuild_fail", "rebuild failed");
        return false;
    }
}

void restart_self(EventBus& bus) {
    bus.emit("restart", "restarting engine");

#ifdef _WIN32
    system("build\\aniva_crash.exe");
#else
    system("./build/aniva_crash");
#endif
}

int main() {
    EventBus bus;
    LiveDiagnostics diag(bus);

    if (sources_changed()) {
        bus.emit("rebuild", "sources changed → rebuilding");

        if (!rebuild(bus)) {
            bus.emit("fatal", "rebuild failed, aborting");
            return 1;
        }

        restart_self(bus);
        return 0;
    }

    bus.emit("start", "starting hunt loop");

    HuntLoop loop(bus);
    loop.run();
}
