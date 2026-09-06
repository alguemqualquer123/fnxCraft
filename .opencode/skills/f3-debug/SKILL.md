---
name: f3-debug
description: Use when creating or modifying the F3 debug overlay like Minecraft - shows FPS, position, chunk, biome, facing, block info. Triggers on "F3", "debug overlay", "mostrar informações".
---

# F3 Debug Skill

Implementa overlay estilo Minecraft ativado com F3.

## Passos

1. **Estado** - `GameData` em `src/gameLayer/gamePlayLogic.cpp`:
   ```cpp
   bool showF3Debug = false;
   ```
   Toggle em `isKeyReleased(Button::F3)` 

2. **Coleta de dados** - calcular no frame:
   - FPS: `programData.currentFps`
   - Pos: `player.entity.position` (dvec3)
   - BlockPos: `from3DPointToBlock(position)`
   - Chunk: `blockPos / CHUNK_SIZE`
   - Facing: `c.viewDirection` + `getViewDirectionRotation()` (0=-Z,1=-X,2=+Z,3=+X)
   - Biome: `biomesManager` ou `worldGenerator`
   - Luz: `block->getSkyLight()/getLight()`
   - Entidades: `entityManager` counts
   - Mem/GPU se disponível

3. **Render** - em `renderGameUI` ou após `renderFromBakedData`:
   ```cpp
   if(gameData.showF3Debug){
     renderer2d.renderText({10,10}, text, font, Colors_White, 12);
     // ou ImGui::Begin("F3 Debug", nullptr, ImGuiWindowFlags_NoDecoration)
   }
   ```
   Usar `programData.ui.renderer2d` com fonte monoespaçada, fundo semi-transparente

4. **Formato texto** - 2 colunas como Minecraft:
   ```
   Minecraft 1.21 (fnxCraft) / FPS: 60
   XYZ: 123 / 64 / -45
   Block: 123 64 -45
   Chunk: 7  -3  in 8 8
   Facing: north (Towards negative Z)
   Biome: Plains
   Light: sky 15 block 7
   Entities: 12
   ```

5. **Build** - `cmake --build build -j4`
