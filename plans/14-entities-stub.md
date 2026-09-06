# [🔴 Avançado] 36 Entidades Stub → Implementar

## Objetivo
Implementar `.cpp` para 36 entidades que hoje só têm header.

## Checklist
- [ ] `Creeper` + `CaveSpider` — hostile básico
- [ ] `CrystalSentinel/Bat/Golem` — cristal family
- [ ] `Slime` / `Shlime`
- [ ] `QueenBee` + `Bee` + `HoneyBear`
- [ ] `HermitCrab` + `Manatee`
- [ ] `LightFairy` + `JuvenileDragon` + `Hydra` + `TreeEnt`
- [ ] `MimicChest` + `ArmoredBoar` + `MistGhost` + `Enderling`
- [ ] `Cow` + `Sheep` + `Wolf` + `Fox` + `Crow`
- [ ] `Skeleton` + `SkeletonPirate` + `StoneGolem` + `LavaSlug` + `SandSerpent` + `RiverGuardian`
- [ ] Villagers: `Blacksmith` + `Herbalist` + `NomadTrader` + `CapybaraChef`
- [ ] `fishing` system
- [ ] Cada um: `src/gameLayer/gameplay/<nome>.cpp` + AI + loot + spawn

## Arquivos
- `include/gameLayer/gameplay/*.h` — 66 headers (30 com cpp, 36 stub)
- `src/gameLayer/gameplay/` — criar 36 `.cpp`
- `include/gameLayer/gameplay/allentities.h` — registrar

## Como Testar
Spawnar cada entidade via `spawnEgg` e verificar AI/animação.

## Prioridade
🔴 Avançado — 36 stubs, fazer incremental 1 por vez
