# Scripts - ourCraft

Atalhos para não digitar comandos manuais.

```sh
./scripts/configure.sh   # cmake -B build -S . -DCMAKE_POLICY_VERSION_MINIMUM=3.5
./scripts/build.sh       # cmake --build build --target ourCraft -j$(nproc)
./scripts/build-all.sh   # cmake --build build -j$(nproc)
./scripts/run.sh         # ./build/ourCraft | tee game_runtime.log
./scripts/run-console.sh # gnome-terminal + tee
./scripts/run-server.sh  # ./build/ourCraftServer
./scripts/clean.sh       # rm -rf build
./scripts/dev.sh         # configure + build + run (Debug)
```

Todos com `chmod +x` e `set -e`, usam `DIR=$(cd $(dirname $0)/.. && pwd)`.
