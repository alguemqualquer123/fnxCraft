#!/bin/bash
DIR="$(cd "$(dirname "$0")/.." && pwd)"
cd "$DIR"
./build/fnxCraft 2>&1 | tee game_runtime.log
