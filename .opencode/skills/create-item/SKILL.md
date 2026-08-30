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
7. **Textura PNG** - Gerar `resources/assets/items/<path>` 16x16 RGBA via Python PIL. Estilo: contorno preto, sombreamento, transparente. Usar `mkdir -p` e copiar se precisar de variação de cor.
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

## Notas

- `lastItem` é sentinela, nunca remover
- Texturas são carregadas em `src/gameLayer/blocksLoader.cpp` via `getItemTextureName`
- Case-sensitive no Linux: `ChestPlate` vs `Chestplate` quebra
- Sempre rodar build após editar
