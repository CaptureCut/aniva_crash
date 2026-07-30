#pragma once
#include <string>
#include "bus/event_bus.h"
#include "gpu_runtime.h"

struct GpuBiasResult {
    float coverage_score;
    float pattern_score;
    float crash_score;
    float total_bias;
};

class GpuBias {
public:
    GpuBias(EventBus& bus, GpuRuntime& runtime);

    GpuBiasResult compute(const std::string& js_code);

private:
    EventBus& bus_;
    GpuRuntime& runtime_;
};
