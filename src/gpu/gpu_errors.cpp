// src/gpu/gpu_errors.cpp
#include "gpu_errors.h"
#include <iostream>

std::string gpu_error_to_string(cudaError_t err) {
    return std::string(cudaGetErrorString(err));
}

bool gpu_check(cudaError_t err, const char* msg) {
    if (err == cudaSuccess)
        return true;

    std::cerr << "[GPU_ERROR] " << msg << ": "
              << cudaGetErrorString(err) << std::endl;

    return false;
}
