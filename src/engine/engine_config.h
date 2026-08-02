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

    // секция sandbox (опциональная)
    struct Sandbox {
        bool seccomp = false;
        bool cgroups = false;
        int memory_limit_mb = 0;
        int cpu_limit_percent = 0;
    } sandbox;

    // секция генератора (опциональная)
    struct Generator {
        int max_tokens = 4096;
        bool templates_enabled = true;
        bool pattern_bias = true;
        bool gpu_bias = true;
    } generator;
};

// загрузка конфигурации из JSON
EngineConfig load_engine_config(const std::string& path);
