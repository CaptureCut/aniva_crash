#pragma once

#include <vector>
#include <string>

#include "bus/event_bus.h"
#include "gpu_runtime.h"
#include "core/pattern/pattern_analyzer.h"   // ← правильный путь

struct GpuBiasResult {
    float total_bias = 0.0f;
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
