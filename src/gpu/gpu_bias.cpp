#include "gpu_bias.h"
#include "gpu_runtime.h"
#include "bus/event_bus.h"

GpuBias::GpuBias(EventBus& bus, GpuRuntime& runtime)
    : bus_(bus)
    , runtime_(runtime)
{}

void GpuBias::warmup() {
    bus_.publish("gpu_bias:warmup", "GPU bias warmup started");
    // preload CUDA kernels, allocate buffers (future)
    bus_.publish("gpu_bias:warmup_done", "GPU bias warmup finished");
}

void GpuBias::shutdown() {
    bus_.publish("gpu_bias:shutdown", "GPU bias shutdown started");
    // free GPU resources (future)
    bus_.publish("gpu_bias:shutdown_done", "GPU bias shutdown finished");
}

GpuBiasResult GpuBias::compute(const std::string& js_code,
                               const std::vector<PatternHit>& patterns)
{
    bus_.publish("gpu_bias:start", "GPU bias computation started");

    //
    // 1) CPU-side bias
    //
    float cpu_bias = 0.0f;
    for (const auto& hit : patterns) {
        cpu_bias += hit.weight;
    }

    //
    // 2) GPU-side bias (stub)
    //
    auto flag = [&](bool cond) { return cond ? 1 : 0; };

    std::vector<int> crashFlags{ flag(js_code.find("throw") != std::string::npos) };
    std::vector<int> scoreFlags{ static_cast<int>(js_code.size() % 7) };
    std::vector<int> tfFlags{ flag(js_code.find("for") != std::string::npos) };
    std::vector<int> mgFlags{ flag(js_code.find("map") != std::string::npos) };
    std::vector<int> wasmFlags{ flag(js_code.find("WebAssembly") != std::string::npos) };
    std::vector<int> taFlags{ flag(js_code.find("try") != std::string::npos) };
    std::vector<int> gcFlags{ flag(js_code.find("gc") != std::string::npos) };

    GpuRuntimeResult gpu_res = runtime_.score(
        crashFlags,
        scoreFlags,
        tfFlags,
        mgFlags,
        wasmFlags,
        taFlags,
        gcFlags
    );

    //
    // 3) Final bias
    //
    GpuBiasResult out{};
    out.total_bias = cpu_bias + gpu_res.total_bias;

    bus_.publish("gpu_bias:value", "bias=" + std::to_string(out.total_bias));
    bus_.publish("gpu_bias:end", "GPU bias computation finished");

    return out;
}
