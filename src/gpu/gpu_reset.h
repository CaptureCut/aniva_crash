#pragma once
#include <cuda_runtime.h>

class GpuReset {
public:
    // Сбрасывает ошибки, синхронизирует устройство
    static void soft_reset();

    // Полный сброс GPU (редко нужен)
    static void hard_reset();
};
