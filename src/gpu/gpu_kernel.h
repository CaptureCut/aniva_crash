#pragma once

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
);

} // extern "C"
