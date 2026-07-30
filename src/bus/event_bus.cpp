#include "event_bus.h"

void EventBus::subscribe(const Listener& listener) {
    listeners_.push_back(listener);
}

void EventBus::emit(const std::string& type, const std::string& payload) {
    Event e{type, payload};

    for (auto& listener : listeners_) {
        listener(e);
    }
}
