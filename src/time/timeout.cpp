#include "timeout.h"
#include <thread>
#include <future>

template<typename Func, typename T>
TimeoutResult<T> Timeout::run(Func func, int timeout_ms) {
    TimeoutResult<T> result;

    std::packaged_task<T()> task(func);
    auto fut = task.get_future();

    std::thread worker(std::move(task));
    worker.detach();

    if (fut.wait_for(std::chrono::milliseconds(timeout_ms)) == std::future_status::timeout) {
        result.status = TimeoutStatus::Timeout;
        result.value = T{};
    } else {
        result.status = TimeoutStatus::Completed;
        result.value = fut.get();
    }

    return result;
}

// explicit instantiation for linker
template TimeoutResult<int> Timeout::run<std::function<int()>, int>(std::function<int()>, int);
