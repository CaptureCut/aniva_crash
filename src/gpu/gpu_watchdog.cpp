// src/gpu/gpu_watchdog.cpp
#include "gpu_watchdog.h"
#include <iostream>

GpuWatchdog::GpuWatchdog(EventBus& bus, const GpuConfig& cfg)
    : bus_(bus)
    , cfg_(cfg)
{}

void GpuWatchdog::start() {
    running_ = true;
    worker_ = std::thread(&GpuWatchdog::loop, this);

    bus_.publish("gpu_watchdog:start", "GPU watchdog started");
}

void GpuWatchdog::stop() {
    running_ = false;
    if (worker_.joinable())
        worker_.join();

    bus_.publish("gpu_watchdog:stop", "GPU watchdog stopped");
}

void GpuWatchdog::notify_gpu_start() {
    gpu_busy_ = true;
}

void GpuWatchdog::notify_gpu_end() {
    gpu_busy_ = false;
}

void GpuWatchdog::loop() {
    using namespace std::chrono;

    while (running_) {
        std::this_thread::sleep_for(milliseconds(100));

        if (!gpu_busy_)
            continue;

        // GPU занято — проверяем таймаут
        static auto last_busy_time = steady_clock::now();
        auto now = steady_clock::now();

        auto elapsed = duration_cast<milliseconds>(now - last_busy_time).count();

        if (elapsed > cfg_.watchdog_timeout_ms) {
            bus_.publish("gpu_watchdog:timeout",
                         "GPU timeout detected — performing hard reset");

            std::cerr << "[GPU_WATCHDOG] GPU timeout, resetting device" << std::endl;

            GpuReset::hard_reset();
            gpu_busy_ = false;
        }
    }
}
