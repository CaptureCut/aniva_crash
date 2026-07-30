// src/gpu/gpu_allocator.h
#pragma once
#include <cstddef>

class GpuAllocator {
public:
    GpuAllocator();
    ~GpuAllocator();

    void* alloc(size_t bytes);
    void free(void* ptr);
    void copy_to_device(void* dst, const void* src, size_t bytes);
    void copy_to_host(void* dst, const void* src, size_t bytes);
};
