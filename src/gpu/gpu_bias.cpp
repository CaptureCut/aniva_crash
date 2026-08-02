#include "gpu_bias.h"
#include "gpu_runtime.h"
#include "bus/event_bus.h"

GpuBias::GpuBias(EventBus& bus, GpuRuntime& runtime)
    : bus_(bus)
    , runtime_(runtime)
{}

void GpuBias::warmup() {
    bus_.publish("gpu_bias:warmup", "GPU bias warmup started");
    bus_.publish("gpu_bias:warmup_done", "GPU bias warmup finished");
}

void GpuBias::shutdown() {
    bus_.publish("gpu_bias:shutdown", "GPU bias shutdown started");
    bus_.publish("gpu_bias:shutdown_done", "GPU bias shutdown finished");
}

GpuBiasResult GpuBias::compute(const std::string& js_code,
                               const std::vector<PatternHit>& patterns)
{
    bus_.publish("gpu_bias:start", "GPU bias computation started");

    // CPU-side pattern score
    float cpu_score = 0.0f;
    for (const auto& hit : patterns)
        cpu_score += hit.weight;

    // GPU-side score (new interface)
    GpuRuntimeResult gpu_res = runtime_.score(js_code, patterns);

    // Final bias
    GpuBiasResult out{};
    out.coverage_score = gpu_res.coverage_score;
    out.pattern_score  = gpu_res.pattern_score + cpu_score * 0.5f;
    out.crash_score    = gpu_res.crash_score;

    out.total_bias =
          out.coverage_score * 0.4f
        + out.pattern_score  * 0.4f
        + out.crash_score    * 0.2f;

    bus_.publish("gpu_bias:value", "bias=" + std::to_string(out.total_bias));
    bus_.publish("gpu_bias:end", "GPU bias computation finished");

    return out;
}
