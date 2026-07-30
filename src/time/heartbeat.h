#pragma once

#include <atomic>
#include <thread>
#include <chrono>
#include "telemetry/telemetry.h"

class Heartbeat {
public:
    Heartbeat(Telemetry& telemetry, int interval_ms = 1000);
    ~Heartbeat();

    void start();
    void stop();

    // ручной тик для HuntLoop
    void tick(std::size_t iteration);

private:
    void loop();

    Telemetry& telemetry_;
    int interval_ms_;
    std::atomic<bool> running_;
    std::thread worker_;
};
