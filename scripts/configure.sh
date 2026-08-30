#!/bin/bash
set -e
DIR="$(cd "$(dirname "$0")/.." && pwd)"
cd "$DIR"
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
echo "✓ Configure concluído"
