#include "gpu_bias.h"
#include "gpu_runtime.h"
#include "bus/event_bus.h"

GpuBias::GpuBias(EventBus& bus, GpuRuntime& runtime)
    : bus_(bus)
    , runtime_(runtime)
{}

GpuBiasResult GpuBias::compute(const std::string& js_code) {
    bus_.emit("gpu_bias_start", "gpu bias computation started");

    // отправляем JS-код на GPU
    GpuRuntimeResult gpu_res = runtime_.score(js_code);

    GpuBiasResult out;

    // простая логика:
    // GPU возвращает набор метрик, мы превращаем их в bias
    out.coverage_score = gpu_res.coverage_score;
    out.pattern_score  = gpu_res.pattern_score;
    out.crash_score    = gpu_res.crash_score;

    // итоговый bias
    out.total_bias = 
          out.coverage_score * 0.5
        + out.pattern_score  * 0.3
        + out.crash_score    * 0.2;

    bus_.emit("gpu_bias_value", "bias = " + std::to_string(out.total_bias));
    bus_.emit("gpu_bias_end", "gpu bias computation finished");

    return out;
}
