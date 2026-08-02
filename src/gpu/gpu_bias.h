#pragma once

#include <vector>
#include <string>

#include "bus/event_bus.h"
#include "gpu_runtime.h"
#include "core/pattern/pattern_analyzer.h"

struct GpuBiasResult {
    float coverage_score = 0.0f;   // длина кода, сложность
    float pattern_score  = 0.0f;   // паттерны (CPU + GPU)
    float crash_score    = 0.0f;   // опасные конструкции
    float total_bias     = 0.0f;   // итоговый bias
};

class GpuBias {
public:
    GpuBias(EventBus& bus, GpuRuntime& runtime);

    void warmup();
    void shutdown();

    GpuBiasResult compute(const std::string& js_code,
                          const std::vector<PatternHit>& patterns);

private:
    EventBus& bus_;
    GpuRuntime& runtime_;
};
