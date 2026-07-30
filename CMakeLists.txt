cmake_minimum_required(VERSION 3.20)

project(aniva_crash LANGUAGES CXX CUDA)

# -----------------------------
# CUDA
# -----------------------------
set(CMAKE_CUDA_COMPILER /usr/local/cuda/bin/nvcc)
set(CMAKE_CUDA_ARCHITECTURES 86)

set(CMAKE_CUDA_STANDARD 17)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)

# -----------------------------
# C++
# -----------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_compile_options(
    $<$<COMPILE_LANGUAGE:CXX>:-Wall>
    $<$<COMPILE_LANGUAGE:CXX>:-Wextra>
    $<$<COMPILE_LANGUAGE:CXX>:-Wpedantic>
)

# -----------------------------
# include directories
# -----------------------------
set(PROJECT_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/src)

include_directories(
    ${PROJECT_INCLUDE_DIR}
    /usr/local/cuda/include
)

# -----------------------------
# pattern analyzer (FIX)
# -----------------------------
add_library(core_pattern
    src/core/pattern/pattern_analyzer.cpp
    src/core/pattern/pattern_analyzer.h
)

# -----------------------------
# engine
# -----------------------------
add_library(engine
    src/engine/engine.cpp
    src/engine/engine_config.cpp
)

# -----------------------------
# core: generator
# -----------------------------
add_library(core_generator
    src/core/generator/generator.cpp
    src/core/generator/mutator.cpp
)

# -----------------------------
# core: executor
# -----------------------------
add_library(core_executor
    src/core/executor/executor.cpp
)

# -----------------------------
# core: triage
# -----------------------------
add_library(core_triage
    src/core/triage/triage.cpp
)

# -----------------------------
# core: minimizer
# -----------------------------
add_library(core_minimizer
    src/core/minimizer/minimizer.cpp
)

# -----------------------------
# core: hunt_loop
# -----------------------------
add_library(core_hunt_loop
    src/core/hunt_loop/hunt_loop.cpp
)

# -----------------------------
# bus
# -----------------------------
add_library(bus
    src/bus/event_bus.cpp
)

# -----------------------------
# diagnostics
# -----------------------------
add_library(diagnostics
    src/diagnostics/live_diagnostics.cpp
)

# -----------------------------
# persistence
# -----------------------------
add_library(persistence
    src/persistence/crash_store.cpp
    src/persistence/anomaly_store.cpp
    src/persistence/corpus_store.cpp
)

# -----------------------------
# memory
# -----------------------------
add_library(memory
    src/memory/arena.cpp
    src/memory/bounded_buffer.cpp
)

# -----------------------------
# isolation
# -----------------------------
add_library(isolation
    src/isolation/process_sandbox.cpp
)

# -----------------------------
# time
# -----------------------------
add_library(time
    src/time/timeout.cpp
    src/time/heartbeat.cpp
)

# -----------------------------
# telemetry
# -----------------------------
add_library(telemetry
    src/telemetry/telemetry.cpp
)

# -----------------------------
# gpu
# -----------------------------
add_library(gpu
    src/gpu/gpu_runtime.cu
    src/gpu/gpu_bias.cpp
    src/gpu/gpu_kernel.cu
    src/gpu/gpu_allocator.cpp
    src/gpu/gpu_watchdog.cpp
    src/gpu/gpu_reset.cpp
    src/gpu/gpu_errors.cpp
)

set_target_properties(gpu PROPERTIES
    CUDA_SEPARABLE_COMPILATION ON
)

target_compile_options(gpu PRIVATE
    $<$<COMPILE_LANGUAGE:CUDA>:-Xcompiler -w>
)

# -----------------------------
# main executable
# -----------------------------
add_executable(aniva_crash
    src/main.cpp
)

target_link_libraries(aniva_crash
    engine
    core_generator
    core_executor
    core_triage
    core_minimizer
    core_hunt_loop
    core_pattern

    bus
    diagnostics
    persistence
    memory
    isolation
    time
    telemetry
    gpu
)

# -----------------------------
# tools: bootstrap
# -----------------------------
add_executable(bootstrap
    tools/bootstrap/main.cpp
    tools/bootstrap/diagnostics.cpp
)

target_include_directories(bootstrap PRIVATE
    ${CMAKE_SOURCE_DIR}/tools/bootstrap
    ${PROJECT_INCLUDE_DIR}
)

set_target_properties(bootstrap PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
