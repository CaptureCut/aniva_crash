#include "time/heartbeat.h"

Heartbeat::Heartbeat(Telemetry& telemetry, int interval_ms)
    : telemetry_(telemetry)
    , interval_ms_(interval_ms)
    , running_(false)
{}

Heartbeat::~Heartbeat() {
    stop();
}

void Heartbeat::start() {
    if (running_) return;
    running_ = true;

    worker_ = std::thread([this]() {
        this->loop();
    });
}

void Heartbeat::stop() {
    if (!running_) return;
    running_ = false;

    if (worker_.joinable())
        worker_.join();
}

void Heartbeat::tick(std::size_t iteration) {
    telemetry_.inc("heartbeat_count", 1);
    telemetry_.set("alive", 1.0);
    telemetry_.set("last_iter", static_cast<double>(iteration));
}

void Heartbeat::loop() {
    using namespace std::chrono;

    while (running_) {
        auto start = high_resolution_clock::now();

        telemetry_.inc("heartbeat_count", 1);
        telemetry_.set("alive", 1.0);

        auto end = high_resolution_clock::now();
        auto dur = duration_cast<microseconds>(end - start);

        telemetry_.add_time("heartbeat_ms", dur);

        std::this_thread::sleep_for(milliseconds(interval_ms_));
    }
}
