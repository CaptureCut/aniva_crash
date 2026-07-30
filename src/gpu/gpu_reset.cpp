// src/gpu/gpu_reset.cpp
#include "gpu_reset.h"
#include <iostream>

void GpuReset::soft_reset() {
    // Сбрасываем ошибки CUDA
    cudaError_t err = cudaGetLastError();
    if (err != cudaSuccess) {
        std::cerr << "[GPU_RESET] Clearing CUDA error: "
                  << cudaGetErrorString(err) << std::endl;
    }

    // Синхронизируем устройство
    err = cudaDeviceSynchronize();
    if (err != cudaSuccess) {
        std::cerr << "[GPU_RESET] cudaDeviceSynchronize failed: "
                  << cudaGetErrorString(err) << std::endl;
    }
}

void GpuReset::hard_reset() {
    // Полный сброс устройства
    cudaError_t err = cudaDeviceReset();
    if (err != cudaSuccess) {
        std::cerr << "[GPU_RESET] cudaDeviceReset failed: "
                  << cudaGetErrorString(err) << std::endl;
    } else {
        std::cout << "[GPU_RESET] GPU device reset successfully" << std::endl;
    }
}
