#include <cuda_runtime.h>

extern "C" {

__global__
void score_kernel(
    const int* __restrict__ crashFlags,
    const int* __restrict__ scoreFlags,
    const int* __restrict__ tfFlags,
    const int* __restrict__ mgFlags,
    const int* __restrict__ wasmFlags,
    const int* __restrict__ taFlags,
    const int* __restrict__ gcFlags,
    float* __restrict__ outScores,
    int count
) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx >= count) return;

    float score = 0.0f;

    score += crashFlags[idx] * 10.0f;
    score += scoreFlags[idx] * 5.0f;
    score += tfFlags[idx]   * 1.5f;
    score += mgFlags[idx]   * 2.0f;
    score += wasmFlags[idx] * 3.0f;
    score += taFlags[idx]   * 1.0f;
    score += gcFlags[idx]   * 2.5f;

    // усиливаем различимость
    score = score * score;

    outScores[idx] = score;
}

} // extern "C"
