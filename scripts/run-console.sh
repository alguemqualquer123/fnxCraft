#!/bin/bash
DIR="$(cd "$(dirname "$0")/.." && pwd)"
cd "$DIR"
gnome-terminal -- bash -c '
echo "=== ourCraft debug console ==="
./build/ourCraft 2>&1 | tee game_runtime.log
echo "=== Game exited (code: ${PIPESTATUS[0]}) ==="
read
'
