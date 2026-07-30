#pragma once
#include <string>
#include <functional>
#include <vector>

class EventBus {
public:
    struct Event {
        std::string type;
        std::string payload;
    };

    using Listener = std::function<void(const Event&)>;

    void subscribe(const Listener& listener);

    // основной метод
    void emit(const std::string& type, const std::string& payload);

    // совместимость со старым API
    void publish(const std::string& type, const std::string& payload) {
        emit(type, payload);
    }

private:
    std::vector<Listener> listeners_;
};
