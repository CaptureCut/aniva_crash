#pragma once
#include <string>

struct EngineConfig {
    // путь к d8
    std::string engine_path;

    // директории корпуса и крашей
    std::string corpus_dir;
    std::string crash_dir;

    // таймаут исполнения JS
    int timeout_ms = 200;

    // период heartbeat
    int heartbeat_ms = 1000;

    // путь к GPU‑конфигу (опционально)
    std::string gpu_config;
};

// загрузка конфигурации из JSON
EngineConfig load_engine_config(const std::string& path);
