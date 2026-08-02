#include "gpu_bias.h"
#include "gpu_runtime.h"
#include "bus/event_bus.h"

GpuBias::GpuBias(EventBus& bus, GpuRuntime& runtime)
    : bus_(bus)
    , runtime_(runtime)
{}

GpuBiasResult GpuBias::compute(const std::string& js_code,
                               const std::vector<PatternHit>& patterns)
{
    bus_.publish("gpu_bias:start", "GPU bias computation started");

    // GPU анализирует JS-код
    GpuRuntimeResult gpu_res = runtime_.score(js_code);

    // CPU анализирует паттерны
    float cpu_score = 0.0f;
    float danger_score = 0.0f;

    for (const auto& p : patterns) {
        cpu_score += p.weight;

        switch (p.kind) {
            case PatternKind::WasmOob:
                danger_score += 1.2f;
                break;
            case PatternKind::TypedArrayOob:
                danger_score += 1.0f;
                break;
            case PatternKind::ProxyRec:
                danger_score += 1.0f;
                break;
            case PatternKind::RegExpCat:
                danger_score += 0.9f;
                break;
            case PatternKind::JitDeopt:
                danger_score += 0.7f;
                break;
            case PatternKind::GcPressure:
                danger_score += 0.6f;
                break;
            default:
                danger_score += p.weight * 0.2f;
                break;
        }
    }

    GpuBiasResult out;

    out.coverage_score = gpu_res.coverage_score;
    out.pattern_score  = gpu_res.pattern_score + cpu_score * 0.4f;
    out.crash_score    = gpu_res.crash_score;

    // итоговый bias
    out.total_bias =
          out.coverage_score * 0.3f
        + out.pattern_score  * 0.3f
        + out.crash_score    * 0.2f
        + danger_score       * 0.2f;

    // нормализация
    if (out.total_bias > 1.5f)
        out.total_bias = 1.5f;

    bus_.publish("gpu_bias:value", "bias=" + std::to_string(out.total_bias));
    bus_.publish("gpu_bias:end", "GPU bias computation finished");

    return out;
}
