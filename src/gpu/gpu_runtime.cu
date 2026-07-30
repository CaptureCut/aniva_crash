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

GpuRuntimeResult GpuRuntime::score(
    const std::vector<int>& crash,
    const std::vector<int>& score,
    const std::vector<int>& tf,
    const std::vector<int>& mg,
    const std::vector<int>& wasm,
    const std::vector<int>& ta,
    const std::vector<int>& gc
) {
    GpuRuntimeResult result{};

    int count = crash.size();
    if (count == 0) {
        return result;
    }

    // device buffers
    int *d_crash = nullptr, *d_score = nullptr, *d_tf = nullptr;
    int *d_mg = nullptr, *d_wasm = nullptr, *d_ta = nullptr, *d_gc = nullptr;
    float* d_out = nullptr;

    auto alloc = [&](auto** ptr, size_t bytes, const char* name) {
        return check(name, cudaMalloc(ptr, bytes));
    };

    alloc(&d_crash, count * sizeof(int), "malloc crash");
    alloc(&d_score, count * sizeof(int), "malloc score");
    alloc(&d_tf,    count * sizeof(int), "malloc tf");
    alloc(&d_mg,    count * sizeof(int), "malloc mg");
    alloc(&d_wasm,  count * sizeof(int), "malloc wasm");
    alloc(&d_ta,    count * sizeof(int), "malloc ta");
    alloc(&d_gc,    count * sizeof(int), "malloc gc");
    alloc(&d_out,   count * sizeof(float), "malloc out");

    auto copy = [&](auto* dst, const auto& src, const char* name) {
        return check(name, cudaMemcpy(dst, src.data(),
                                      src.size() * sizeof(int),
                                      cudaMemcpyHostToDevice));
    };

    copy(d_crash, crash, "copy crash");
    copy(d_score, score, "copy score");
    copy(d_tf,    tf,    "copy tf");
    copy(d_mg,    mg,    "copy mg");
    copy(d_wasm,  wasm,  "copy wasm");
    copy(d_ta,    ta,    "copy ta");
    copy(d_gc,    gc,    "copy gc");

    dim3 block(256);
    dim3 grid((count + block.x - 1) / block.x);

    score_kernel<<<grid, block>>>(
        d_crash, d_score, d_tf, d_mg, d_wasm, d_ta, d_gc,
        d_out, count
    );

    check("kernel launch", cudaGetLastError());
    check("device sync", cudaDeviceSynchronize());

    std::vector<float> out(count);
    check("copy out", cudaMemcpy(out.data(), d_out,
                                 count * sizeof(float),
                                 cudaMemcpyDeviceToHost));

    cudaFree(d_crash);
    cudaFree(d_score);
    cudaFree(d_tf);
    cudaFree(d_mg);
    cudaFree(d_wasm);
    cudaFree(d_ta);
    cudaFree(d_gc);
    cudaFree(d_out);

    float sum = 0.0f;
    for (float v : out) sum += v;

    result.total_bias = sum;
    return result;
}
