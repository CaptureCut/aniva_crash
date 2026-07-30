#include "telemetry.h"
#include <sstream>

Telemetry::Telemetry() {}

void Telemetry::inc(const std::string& key, int amount) {
    int_metrics_[key] += amount;
}

void Telemetry::set(const std::string& key, double value) {
    double_metrics_[key] = value;
}

void Telemetry::add_time(const std::string& key, std::chrono::microseconds us) {
    time_metrics_ms_[key] += us.count() / 1000.0;
}

int Telemetry::get_int(const std::string& key) const {
    auto it = int_metrics_.find(key);
    return it == int_metrics_.end() ? 0 : it->second;
}

double Telemetry::get_double(const std::string& key) const {
    auto it = double_metrics_.find(key);
    return it == double_metrics_.end() ? 0.0 : it->second;
}

double Telemetry::get_time_ms(const std::string& key) const {
    auto it = time_metrics_ms_.find(key);
    return it == time_metrics_ms_.end() ? 0.0 : it->second;
}

std::string Telemetry::dump_json() const {
    std::ostringstream oss;
    oss << "{";

    bool first = true;

    for (auto& [k, v] : int_metrics_) {
        if (!first) oss << ",";
        first = false;
        oss << "\"" << k << "\":" << v;
    }

    for (auto& [k, v] : double_metrics_) {
        if (!first) oss << ",";
        first = false;
        oss << "\"" << k << "\":" << v;
    }

    for (auto& [k, v] : time_metrics_ms_) {
        if (!first) oss << ",";
        first = false;
        oss << "\"" << k << "\":" << v;
    }

    oss << "}";
    return oss.str();
}
