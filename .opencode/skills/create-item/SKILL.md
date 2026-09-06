---
name: create-item
description: Use when creating a new Minecraft-like item - handles enum, textures, 3D model name, localization, description, crafting recipe, and auto-generates 16x16 pixel-art PNG. Triggers on "criar item", "novo item", "add item".
---

# Create Item Skill

Quando o usuário pedir para criar um item, execute o checklist completo. Nunca crie apenas a textura ou apenas o enum.

## Checklist obrigatório

1. **Enum** - `include/gameLayer/gameplay/items.h` -> adicionar antes de `lastItem`
2. **Texturas** - `src/gameLayer/gameplay/items.cpp` arrays:
   - `itemsNamesTextures[]` (path relativo a `resources/assets/items/`, ex `bows/bow.png`)
   - `item3DModelName[]` (nome sem extensão)
   - `itemsNames[]` (nome display)
3. **Descrição** - `src/gameLayer/gameplay/itemDescription.cpp` -> `getItemDescription()` switch
4. **Localização** - `src/gameLayer/localization.cpp` -> adicionar em `loc_ItemName` e dicionário PT
5. **Lógica** - `src/gameLayer/gameplay/items.cpp` implementar helpers se necessário:
   - `isBow()`, `isAmmo()`, `isWeapon()`, `getStackSize()`, `isEatable()` etc
   - Atualizar `getItemStats()` / `getWeaponStats()` se for arma/ferramenta/armadura
6. **Crafting** - `src/gameLayer/gameplay/crafting.cpp` -> adicionar receita
7. **Textura PNG — 16x16 ESTRITO** - Gerar `resources/assets/items/<path>` **16x16 RGBA 8-bit** via Python PIL. `loadFromFileAndAddPadding` + `loadItem2D` exigem exatamente 16x16, `a>1` para corte de mesh (thick=1/8). Qualquer outro tamanho = fallback checker rosa `146,52,235` ou bleed. Estilo: contorno preto, sombreamento, fundo transparente `(0,0,0,0)`. Spawn eggs custom: `spawnEggs/<mob>.png` 16x16; tint fallback exige `spawn_egg.png` e `spawn_egg_overlay.png` mesmo size (checado `blocksLoader.cpp:1992`). Usar `mkdir -p`
8. **Modelo 3D** (se item for arma/ferramenta) - verificar `resources/assets/models/` ou usar modelo existente `human` hand
9. **Verificação** - `grep` count deve bater `lastItem - ItemsStartPoint` nos 3 arrays, `cmake --build build -j4` sem erro
10. **Teste in-game** - `/give` ou inventário deve mostrar item com nome/descrição e textura carregada sem fallback rosa

## Template Python para textura

```python
from PIL import Image, ImageDraw
im=Image.new("RGBA",(16,16),(0,0,0,0))
d=ImageDraw.Draw(im)
d.rectangle([4,4,11,11], fill=(180,120,40,255), outline=(0,0,0,255))
im.save("resources/assets/items/<path>.png","PNG")
```

## Exemplo: arco

Enum: `woodenBow`, Textura: `bows/bow.png`, Model: `bow`, Stack 1, isBow() retorna true, receita: `stick + string/cloth`

## Consumo e uso

- `isItemThatCanBeUsed()` + `isConsumedAfterUse()` em `items.cpp:18/31` — adicionar ovo/item em AMBOS. Consumo só `SURVIVAL` (`gamePlayLogic.cpp:1589/1679` e `tick.cpp:1761`), `CREATIVE` não consome.
- Clique no chão (`RMB`) envia `Packet_ClientUsedItem` com `raycast → blockToPlace`; servidor valida `revisionNumber` + `type==i.t.itemType` e `allowed` antes de `counter--`. Se falhar, `sendPlayerInventoryAndIncrementRevision` restaura.
- Spawn eggs SEM handler em `tick.cpp:1594` somem sem spawnar — sempre adicionar `else if(from->type==ItemTypes::myMobSpawnEgg){ spawn... }`.

## Modelo 3D e shader

- `item3DModelName[]` → `resources/assets/models/items/<nome>.glb` (Assimp `Triangulate|GenNormals|EmbedTextures`), `loadModel3D` (`blocksLoader.cpp:2497`) extrai difuso embutido + `glGetTextureHandleARB` bindless, fallback `checker`.
- Shader `itemEntity.frag` / `basicEntity.vert` — PBR, `#version 430→410` Mac, `GLAD_GL_ARB_bindless_texture`.

## Notas

- `lastItem` é sentinela, nunca remover
- Texturas são carregadas em `src/gameLayer/blocksLoader.cpp` via `getItemTextureName` + `loadFromFileAndAddPadding` (borda 1px)
- 16x16 estrito — outro tamanho quebra mesh e gera rosa
- Case-sensitive no Linux: `ChestPlate` vs `Chestplate` quebra
- Sempre rodar `cmake --build build -j4` após editar e testar `/give myMobSpawnEgg` + `E`/`RMB` no chão
