#!/bin/bash
set -e
DIR="$(cd "$(dirname "$0")/.." && pwd)"
cd "$DIR"
cmake --build build --target fnxCraft -j$(nproc)
echo "✓ Build fnxCraft concluído"
