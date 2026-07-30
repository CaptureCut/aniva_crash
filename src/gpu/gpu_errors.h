#pragma once
#include <cuda_runtime.h>
#include <string>

std::string gpu_error_to_string(cudaError_t err);
bool gpu_check(cudaError_t err, const char* msg);
