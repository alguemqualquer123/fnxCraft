#!/bin/bash
set -e
DIR="$(cd "$(dirname "$0")/.." && pwd)"
cd "$DIR"
cmake --build build --target ourCraft -j$(nproc)
echo "✓ Build ourCraft concluído"
