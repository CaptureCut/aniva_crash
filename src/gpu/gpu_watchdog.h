#pragma once

#include <atomic>
#include <chrono>
#include <thread>
#include <string>

#include "gpu_config.h"
#include "gpu_reset.h"
#include "bus/event_bus.h"

class GpuWatchdog {
public:
    GpuWatchdog(EventBus& bus, const GpuConfig& cfg);

    // Запуск мониторинга
    void start();

    // Остановка мониторинга
    void stop();

    // Сообщить watchdog'у, что GPU занято
    void notify_gpu_start();

    // Сообщить watchdog'у, что GPU освободилось
    void notify_gpu_end();

private:
    void loop();

    EventBus& bus_;
    const GpuConfig& cfg_;

    std::atomic<bool> running_{false};
    std::atomic<bool> gpu_busy_{false};

    std::thread worker_;
};
