# Implementation Plans

This directory contains the organization of implementation plans for the ourCraft project.

## Directory Structure

```
plans/
├── README.md                    # This file
├── COMPLETED-IMPLEMENTED.md     # Inventário completo por pastas (game/server/data/...)
├── completed/                   # Planos concluídos (movidos de pending/future)
├── in-progress/                 # Planos parcialmente feitos
├── failed/                      # Falhas corrigidas parcialmente
├── pending/                     # Aguardando
└── future/                      # Roadmap futuro
```

## File Naming Convention

Use the format: `[category]-[short-description].md`

Examples:
- `rendering-skybox-refactor.md`
- `server-player-sync.md`
- `gameplay-survival-mode.md`

## Categories

- **rendering**: Graphics, shaders, visual effects
- **server**: Multiplayer, networking, synchronization
- **gameplay**: Game mechanics, player interactions
- **audio**: Sounds, music, audio effects
- **worldgen**: World generation, terrain, biomes
- **ui**: User interface, menus, HUD
- **performance**: Optimization, memory, profiling
- **bugfix**: Bug fixes and patches

## Status Badges

Use these badges in your plan files:

- `[IN PROGRESS]` - Currently being worked on
- `[PENDING]` - Waiting to start
- `[FAILED]` - Implementation failed
- `[COMPLETED]` - Successfully implemented
- `[BLOCKED]` - Blocked by dependency

## From Existing TODOs

The following tasks have been migrated from existing TODO files:

### From todo.txt
- Entity visibility in chunks
- Road appearance improvements
- Sprite migration
- View distance optimization
- World generation improvements
- Dimension loading
- UV shrinking
- Folder restructuring

### From hardertodos.md
- Server message optimization
- Player sync on join
- Server state persistence
- Cleanup on exit
- Transparent geometry caching
- Skybox refactor
- Player/zombie rendering
- Line drawing
- Camera fixes
- Item dropping/survival mode
- Player collision

### From soundsTodo.md
- Water sounds
- Block interaction sounds
- UI sounds

### From Known Bugs I've Seen
- B001: Stepping sounds through walls
- B002: Glass block break sound
