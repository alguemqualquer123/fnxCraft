# Scripts - fnxCraft

Atalhos para não digitar comandos manuais.

```sh
./scripts/configure.sh   # cmake -B build -S . -DCMAKE_POLICY_VERSION_MINIMUM=3.5
./scripts/build.sh       # cmake --build build --target fnxCraft -j$(nproc)
./scripts/build-all.sh   # cmake --build build -j$(nproc)
./scripts/run.sh         # ./build/fnxCraft | tee game_runtime.log (abre console externo automaticamente + splashscreen)
./scripts/run-console.sh # gnome-terminal + tee (console dedicado)
./scripts/open-console.sh # abre console externo tail -f logs/game.log (Linux: gnome-terminal/konsole/xterm)
./scripts/run-server.sh  # ./build/fnxCraftServer
./scripts/clean.sh       # rm -rf build
./scripts/dev.sh         # configure + build + run (Debug)
```

Splashscreen: fullscreen ImGui 55%w 18h roxo-azul, progresso 0-100% com modulos (audio, texturas, rede, Lua) + 380ms final. Console externo abre automaticamente ao iniciar o game (Linux) via `glfwMain` tail -f logs/game.log. Desative com `FNXCRAFT_NO_CONSOLE=1 ./build/fnxCraft`.

Todos com `chmod +x` e `set -e`, usam `DIR=$(cd $(dirname $0)/.. && pwd)`.
