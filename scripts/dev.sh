#!/bin/bash
set -e
DIR="$(cd "$(dirname "$0")/.." && pwd)"
cd "$DIR"
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target fnxCraft -j$(nproc)
./build/fnxCraft 2>&1 | tee game_runtime.log
