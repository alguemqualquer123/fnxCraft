#!/bin/bash
gnome-terminal -- bash -c '
cd /home/srvinix/Documentos/ourCraft
echo "=== ourCraft debug console ==="
echo ""
./build/ourCraft 2>&1 | tee game_runtime.log
EXIT_CODE=${PIPESTATUS[0]}
echo ""
echo "=== Game exited (code: $EXIT_CODE) ==="
echo "Errors saved to game_runtime.log"
echo "Press Enter to close..."
read
'
