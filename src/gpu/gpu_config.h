// src/gpu/gpu_config.h
#pragma once

#include <cstddef>
#include <string>

struct GpuConfig {
    // Размеры CUDA-сетки
    int block_size = 256;          // количество потоков в блоке
    int max_blocks = 1024;         // максимальное количество блоков

    // Лимиты памяти
    size_t max_device_alloc = 256 * 1024 * 1024; // 256 MB
    size_t max_host_alloc   = 256 * 1024 * 1024; // 256 MB

    // Тайминги для watchdog
    int watchdog_timeout_ms = 5000;

    // Логи
    bool verbose = false;

    std::string to_string() const {
        return "GpuConfig{ block_size=" + std::to_string(block_size) +
               ", max_blocks=" + std::to_string(max_blocks) +
               ", max_device_alloc=" + std::to_string(max_device_alloc) +
               ", watchdog_timeout_ms=" + std::to_string(watchdog_timeout_ms) +
               ", verbose=" + (verbose ? "true" : "false") +
               " }";
    }
};
