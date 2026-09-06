# [IN PROGRESS] Skybox Refactor

## Overview
Major refactor of the skybox system to fix sun, fog, underwater effects, day/night cycle, and skybox reflections.

## Related Tasks
- Fix sun rendering
- Fix fog effects
- Fix underwater visual effects
- Implement proper day/night cycle
- Fix skybox reflections

## Status
- [ ] Sun rendering fix — pendente (usa `shadingSettings` + `skyBoxRenderer`, precisa `sun.frag` 410 fallback)
- [x] Fog system overhaul — **FEITO 30 Aug 2026**: `shader.cpp:preprocessShaderSource` `#version 430→410` Mac, `glfwMain 4.1/4.6` com `FORWARD_COMPAT`, `[AssetValidator]` loga fallback
- [x] Underwater effects — **FEITO**: `physics.cpp:applyWaterPhysics` + `WATER_VERTICAL_DAMPING`, `cache/` descartável, `logs/` separado
- [ ] Day/night cycle — pendente
- [ ] Skybox reflections — pendente (bindless fallback `!GLAD_GL_ARB_bindless_texture` detectado, mas renderer ainda tenta `glGetTextureHandleARB`)

## Notes
From hardertodos.md: "big refactor for SKYBOX! + fix sun and fog and underwater stuff and day night and skybox reflections"

## Dependencies
- Rendering system
- Shader management

## Priority
High - Core visual feature
