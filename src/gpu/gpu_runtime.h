#pragma once

#include <string>
#include <vector>
#include <cuda_runtime.h>

#include "core/pattern/pattern_analyzer.h"

// результат GPU-анализа
struct GpuRuntimeResult {
    float coverage_score = 0.0f;   // длина кода, сложность
    float pattern_score  = 0.0f;   // паттерны (CPU + GPU)
    float crash_score    = 0.0f;   // опасные конструкции
    float total_bias     = 0.0f;   // итоговый bias
};

class GpuRuntime {
public:
    GpuRuntime() = default;

    // новый интерфейс: GPU анализирует JS-код + паттерны
    GpuRuntimeResult score(const std::string& js_code,
                           const std::vector<PatternHit>& patterns);

private:
    bool check(const char* where, cudaError_t err);
};
