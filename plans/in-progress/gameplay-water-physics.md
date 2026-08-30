# [IN PROGRESS] Water Physics & World Interaction

## Overview
Implement realistic water physics and interaction with the game world, including fluid dynamics, flow, and player/object interactions.

## Related Tasks
- Water flow mechanics (spreading, seeking lower ground)
- Water source block behavior
- Water-structure interaction (filling holes, flowing around blocks)
- Player swimming physics (buoyancy, drag, movement)
- Water pressure and depth effects
- Water rendering updates (caustics, waves, refraction)
- Item/entity buoyancy in water
- Water as light source (underwater lighting)
- Redstone/technology interaction with water

## Status
- [ ] Water flow algorithm (flow direction, distance, spreading) — pendente (fluxo Minecraft)
- [ ] Source block creation and destruction — pendente
- [ ] Water-terrain interaction (filling, flowing) — pendente
- [x] Player buoyancy and swimming mechanics — **FEITO 30 Aug 2026**: `player.cpp: +0.6 tronco`, `physics.cpp:WATER_VERTICAL_DAMPING=2.5`, `ps.gravityModifier 0.3`, `applyWaterPhysics` amortece Y (antes oscilava rápido na superfície)
- [x] Water pressure/drowning system — **FEITO**: `player.isInWater/isSwimming`, `drowningTimer` + `headUnderwater` check
- [x] Entity buoyancy (items, mobs) — **FEITO**: `droppedItem.cpp:+0.15`, `WATER_ITEM_BUOYANCY`, itens giram `2.6rad/s` em cima da água sem tick acelerado
- [ ] Visual effects (surface waves, underwater distortion) — pendente
- [ ] Sound effects integration (splashing, submersion) — pendente (`audio-water-sounds.md`)
- [ ] Water-light interaction (caustics, refraction) — pendente
- [ ] Water-temperature interaction (freezing mechanics?) — pendente

> **Atualizado 30 Aug 2026:** 3/10 concluídos, tick da água corrigido (não mais "acelerado"). Restante fluxo/visuais segue pendente.

## Notes
Current water appears to be static blocks without real physics. Need to implement:
- Minecraft-like fluid dynamics or more advanced simulation
- Proper collision detection with water volumes
- Performance optimization for fluid simulation

## Dependencies
- Block/Chunk system
- Physics engine
- Rendering system (shaders, effects)
- Audio system (water sounds from `audio-water-sounds.md`)
- Player controller

## Priority
High - Core world interaction feature

## Implementation Details
Consider approach:
1. **Simple**: Minecraft-style flowing water (block-based flow, limited distance)
2. **Advanced**: Grid-based fluid simulation with pressure
3. **Complex**: Smooth particle hydrodynamics (SPH) for realistic fluid

Recommended: Start with Minecraft-style for compatibility, then consider enhancements.

## Testing
- Water flows correctly downhill
- Water fills enclosed spaces
- Player can swim and drown
- Items float in water
- Performance remains acceptable with large water volumes

## References
- Minecraft water physics documentation
- Fluid simulation algorithms
- Related to `audio-water-sounds.md` for sound integration
