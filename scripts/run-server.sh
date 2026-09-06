#!/bin/bash
DIR="$(cd "$(dirname "$0")/.." && pwd)"
cd "$DIR"
./build/ourCraftServer 2>&1 | tee server.log
