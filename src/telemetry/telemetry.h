#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <chrono>

class Telemetry {
public:
    Telemetry() = default;

    // counters
    void inc(const std::string& key, uint64_t v = 1);
    void set(const std::string& key, uint64_t value);

    uint64_t get(const std::string& key) const;

    // timings
    void add_time(const std::string& key, std::chrono::microseconds us);

    uint64_t avg_time(const std::string& key) const;
    uint64_t max_time(const std::string& key) const;

private:
    std::unordered_map<std::string, uint64_t> counters_;
    std::unordered_map<std::string, std::vector<uint64_t>> times_;
};
