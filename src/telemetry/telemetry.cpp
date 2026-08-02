#include "telemetry.h"

void Telemetry::inc(const std::string& key, uint64_t v) {
    counters_[key] += v;
}

void Telemetry::set(const std::string& key, uint64_t value) {
    counters_[key] = value;
}

uint64_t Telemetry::get(const std::string& key) const {
    auto it = counters_.find(key);
    return it == counters_.end() ? 0 : it->second;
}

void Telemetry::add_time(const std::string& key, std::chrono::microseconds us) {
    times_[key].push_back(us.count());
}

uint64_t Telemetry::avg_time(const std::string& key) const {
    auto it = times_.find(key);
    if (it == times_.end() || it->second.empty()) return 0;

    uint64_t sum = 0;
    for (auto v : it->second) sum += v;
    return sum / it->second.size();
}

uint64_t Telemetry::max_time(const std::string& key) const {
    auto it = times_.find(key);
    if (it == times_.end() || it->second.empty()) return 0;

    uint64_t m = 0;
    for (auto v : it->second)
        if (v > m) m = v;

    return m;
}
