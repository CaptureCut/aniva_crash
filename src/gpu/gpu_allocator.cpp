// src/gpu/gpu_allocator.cpp
#include "gpu_allocator.h"
#include <cuda_runtime.h>
#include <iostream>

GpuAllocator::GpuAllocator() = default;
GpuAllocator::~GpuAllocator() = default;

void* GpuAllocator::alloc(size_t bytes) {
    void* ptr = nullptr;
    cudaError_t err = cudaMalloc(&ptr, bytes);
    if (err != cudaSuccess) {
        std::cerr << "[GPU_ALLOCATOR] cudaMalloc failed: "
                  << cudaGetErrorString(err) << std::endl;
        return nullptr;
    }
    return ptr;
}

void GpuAllocator::free(void* ptr) {
    if (!ptr) return;
    cudaError_t err = cudaFree(ptr);
    if (err != cudaSuccess) {
        std::cerr << "[GPU_ALLOCATOR] cudaFree failed: "
                  << cudaGetErrorString(err) << std::endl;
    }
}

void GpuAllocator::copy_to_device(void* dst, const void* src, size_t bytes) {
    cudaError_t err = cudaMemcpy(dst, src, bytes, cudaMemcpyHostToDevice);
    if (err != cudaSuccess) {
        std::cerr << "[GPU_ALLOCATOR] copy_to_device failed: "
                  << cudaGetErrorString(err) << std::endl;
    }
}

void GpuAllocator::copy_to_host(void* dst, const void* src, size_t bytes) {
    cudaError_t err = cudaMemcpy(dst, src, bytes, cudaMemcpyDeviceToHost);
    if (err != cudaSuccess) {
        std::cerr << "[GPU_ALLOCATOR] copy_to_host failed: "
                  << cudaGetErrorString(err) << std::endl;
    }
}
