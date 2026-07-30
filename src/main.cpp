#include "engine/engine.h"
#include "engine/engine_config.h"
#include <iostream>

int main() {
    try {
        // загружаем конфиг движка
        EngineConfig cfg = load_engine_config("config/engine_config.json");

        // создаём движок
        Engine engine(cfg);

        // жизненный цикл
        engine.init();
        engine.warmup();
        engine.run();
        engine.shutdown();
    }
    catch (const std::exception& ex) {
        std::cerr << "[FATAL] " << ex.what() << "\n";
        return 1;
    }

    return 0;
}
