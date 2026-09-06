#!/bin/bash
DIR="$(cd "$(dirname "$0")/.." && pwd)"
cd "$DIR"
./build/fnxCraftServer 2>&1 | tee server.log
