#pragma once
#include <string>
#include "bus/event_bus.h"

class LiveDiagnostics {
public:
    LiveDiagnostics(EventBus& bus);

    // обработка события
    void handle(const EventBus::Event& e);

private:
    EventBus& bus_;
};
