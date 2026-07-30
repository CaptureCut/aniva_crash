#include "live_diagnostics.h"
#include <iostream>

LiveDiagnostics::LiveDiagnostics(EventBus& bus)
    : bus_(bus)
{
    // подписываемся на события
    bus_.subscribe([this](const EventBus::Event& e) {
        this->handle(e);
    });
}

void LiveDiagnostics::handle(const EventBus::Event& e) {
    // минимальный живой вывод
    std::cout << "[" << e.type << "] " << e.payload << "\n";
}
