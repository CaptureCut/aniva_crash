#include "gpu_runtime.h"
#include "gpu_kernel.h"
#include <iostream>

bool GpuRuntime::check(const char* where, cudaError_t err) {
    if (err != cudaSuccess) {
        std::cerr << "[GPU] CUDA error at " << where << ": "
                  << cudaGetErrorString(err) << "\n";
        return false;
    }
    return true;
}

GpuRuntimeResult GpuRuntime::score(const std::string& js_code,
                                   const std::vector<PatternHit>& patterns)
{
    GpuRuntimeResult result{};

    // 1) простая эвристика: длина кода → coverage_score
    result.coverage_score = std::min<float>(1.0f, js_code.size() / 5000.0f);

    // 2) паттерны → pattern_score
    float pattern_sum = 0.0f;
    for (const auto& p : patterns)
        pattern_sum += p.weight;

    result.pattern_score = std::min<float>(1.0f, pattern_sum / 5.0f);

    // 3) эвристика crash_score: наличие опасных паттернов
    bool has_wasm = false;
    bool has_oob  = false;
    bool has_proxy = false;

    for (const auto& p : patterns) {
        if (p.kind == PatternKind::Wasm) has_wasm = true;
        if (p.kind == PatternKind::TypedArrayOob) has_oob = true;
        if (p.kind == PatternKind::Proxy) has_proxy = true;
    }

    result.crash_score =
        (has_wasm ? 0.4f : 0.0f) +
        (has_oob  ? 0.4f : 0.0f) +
        (has_proxy? 0.2f : 0.0f);

    // 4) итоговый bias
    result.total_bias =
          result.coverage_score * 0.4f
        + result.pattern_score  * 0.4f
        + result.crash_score    * 0.2f;

    return result;
}
