#!/bin/bash
set -e

echo "[BUILD] Cleaning old build..."
rm -rf build

echo "[BUILD] Configuring..."
cmake -S . -B build

echo "[BUILD] Building..."
cmake --build build -j$(nproc)

echo "[BUILD] Done."
