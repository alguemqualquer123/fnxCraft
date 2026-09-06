#!/bin/bash
set -e
DIR="$(cd "$(dirname "$0")/.." && pwd)"
cd "$DIR"
cmake --build build -j$(nproc)
echo "✓ Build completo"
