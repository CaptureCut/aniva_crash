#pragma once
#include <string>
#include <vector>

#include "bus/event_bus.h"
#include "gpu_runtime.h"
#include "core/pattern/pattern_analyzer.h"

struct GpuBiasResult {
    float coverage_score = 0.0f;   // GPU coverage (basic structural exploration)
    float pattern_score  = 0.0f;   // GPU + CPU pattern score
    float crash_score    = 0.0f;   // GPU crash likelihood
    float total_bias     = 0.0f;   // final combined bias
};

class GpuBias {
public:
    GpuBias(EventBus& bus, GpuRuntime& runtime);

    // GPU + CPU + danger pattern fusion
    GpuBiasResult compute(const std::string& js_code,
                          const std::vector<PatternHit>& patterns);

private:
    EventBus& bus_;
    GpuRuntime& runtime_;
};
