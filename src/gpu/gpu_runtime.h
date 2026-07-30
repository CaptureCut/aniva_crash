#pragma once

#include <vector>
#include <string>
#include <cuda_runtime.h>

struct GpuRuntimeResult {
    float total_bias = 0.0f;
};

class GpuRuntime {
public:
    GpuRuntime() = default;

    GpuRuntimeResult score(
        const std::vector<int>& crash,
        const std::vector<int>& score,
        const std::vector<int>& tf,
        const std::vector<int>& mg,
        const std::vector<int>& wasm,
        const std::vector<int>& ta,
        const std::vector<int>& gc
    );

private:
    bool check(const char* where, cudaError_t err);
};
