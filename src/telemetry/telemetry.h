#pragma once
#include <string>
#include <unordered_map>
#include <chrono>

class Telemetry {
public:
    Telemetry();

    void inc(const std::string& key, int amount = 1);
    void set(const std::string& key, double value);
    void add_time(const std::string& key, std::chrono::microseconds us);

    int get_int(const std::string& key) const;
    double get_double(const std::string& key) const;
    double get_time_ms(const std::string& key) const;

    std::string dump_json() const;

private:
    std::unordered_map<std::string, int> int_metrics_;
    std::unordered_map<std::string, double> double_metrics_;
    std::unordered_map<std::string, double> time_metrics_ms_;
};
