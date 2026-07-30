#pragma once
#include <functional>
#include <chrono>
#include <atomic>

enum class TimeoutStatus {
    Completed,
    Timeout
};

template<typename T>
struct TimeoutResult {
    TimeoutStatus status;
    T value;
};

class Timeout {
public:
    Timeout() = default;

    template<typename Func, typename T = std::invoke_result_t<Func>>
    TimeoutResult<T> run(Func func, int timeout_ms);
};
