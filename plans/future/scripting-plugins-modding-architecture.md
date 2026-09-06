# [PENDING] Script & Plugin Runtime Architecture

## Overview

Complete extensibility system for ourCraft engine enabling Lua scripts, C# scripts, native plugins, mods, hot reload, sandboxing, permissions, and a stable versioned API.

## Status
- [ ] Architecture design (ETAPA 1)
- [ ] Roadmap document (ETAPA 2)
- [ ] API design document (ETAPA 3)
- [ ] Security model document (ETAPA 4)
- [ ] Implementation (ETAPA 5)

## Priority
Low - Long-term future system, not immediate

## Dependencies
- Stable engine core
- ECS system maturity
- Network system stability
- Event system foundation

---

## Architecture Layers

```
┌─────────────────────────────────────────────┐
│                 GAME ENGINE                 │
│                   C++23                     │
│  Renderer / ECS / Physics / Audio / Network │
└──────────────────────┬──────────────────────┘
                       │
                       ▼
┌─────────────────────────────────────────────┐
│            ENGINE PUBLIC API                │
│  Stable ABI / Versioned API / Capability API│
└──────────────────────┬──────────────────────┘
                       │
                       ▼
┌─────────────────────────────────────────────┐
│         SCRIPT & PLUGIN RUNTIME             │
│  Plugin Manager / Event Bus / Scheduler     │
│  Permission System / Sandbox / Memory Mgr   │
└───────┬──────────────┬──────────────┬───────┘
        │              │              │
        ▼              ▼              ▼
┌────────────┐   ┌────────────┐   ┌────────────┐
│ Lua Runtime│   │ .NET / C#  │   │ Native     │
│            │   │ Runtime    │   │ Plugins    │
└────────────┘   └────────────┘   └────────────┘
```

---

## Roadmap Phases

### Phase 0 — Architecture
- Define boundaries
- Define Engine API
- Define handles
- Define ABI
- Define versioning
- Define lifecycle

### Phase 1 — Plugin Core
- Plugin Manager
- Manifest parser
- Lifecycle management
- Dependency Resolver
- Service Registry
- Event Bus

### Phase 2 — Native Plugins
- Dynamic loading (.dll/.so/.dylib)
- Stable C ABI
- Version negotiation
- Crash/error boundaries

### Phase 3 — Lua
- Lua Runtime
- Bindings layer
- Scheduler integration
- Sandbox
- Memory limits
- Hot Reload
- Profiler

### Phase 4 — Resource System
- Client resources
- Server resources
- Shared resources
- Permissions
- Dependencies

### Phase 5 — C#
- .NET Hosting
- Managed API
- Assembly Loading
- Isolation
- Unload
- Hot Reload strategy

### Phase 6 — Runtime Compilation
- Compiler Service
- Background Build
- Incremental Compilation
- Artifact Cache
- Safe Deployment

### Phase 7 — Strong Isolation
- External process execution
- IPC
- Shared memory
- WebAssembly evaluation
- Untrusted mod sandbox

### Phase 8 — SDK
- Documentation
- Templates
- CLI tools
- Plugin Generator
- Debugger
- Profiler
- Package Manager

---

## Plugin Manifest Format (TOML)

```toml
id = "com.example.inventory"
name = "Inventory System"
version = "1.0.0"
runtime = "lua"
entry = "main.lua"
api_version = "1"

[permissions]
world = true
events = true
network = false
filesystem = "resource"

[resources]
memory_limit_mb = 128
cpu_budget_ms = 2
```

---

## Permission Levels

```
Level 0: Trusted Internal Plugin
Level 1: Trusted Third Party Plugin
Level 2: Restricted Plugin
Level 3: Untrusted Mod
Level 4: External Process / Strong Isolation
```

Granular permissions:
- world.read / world.write
- entity.read / entity.create / entity.destroy
- network.send / network.receive / network.rpc
- filesystem.read / filesystem.write
- ui.create / audio.play / server.admin

---

## Safety Mechanisms

- Crash Isolation
- Exception Boundary
- Timeout Detection
- CPU Budget per Plugin
- Memory Budget per Plugin
- Rate Limiting
- Recursive Call Detection
- Quarantine System
- Rollback on Hot Reload failure

---

## Notes

This is a massive future architecture plan. Key decisions:

- **Decision**: Use Handles instead of raw pointers
- **Why**: Prevents dangling pointer bugs, enables validation
- **Alternatives**: Raw pointers, shared_ptr
- **Trade-offs**: Slight indirection overhead vs safety
- **Performance Impact**: Minimal with precomputed dispatch tables
- **Security Impact**: Prevents use-after-free exploits

- **Decision**: Capability-based permissions
- **Why**: Granular control, least privilege principle
- **Alternatives**: Binary trusted/untrusted
- **Trade-offs**: More complex setup vs better security
- **Performance Impact**: Resolved at load time, fast dispatch
- **Security Impact**: Strong isolation boundary

- **Decision**: Separate process isolation for untrusted code
- **Why**: Process boundary is the only true security boundary
- **Alternatives**: In-process sandboxing only
- **Trade-offs**: IPC overhead vs actual security
- **Performance Impact**: Significant for cross-process calls
- **Security Impact**: True isolation

## References

This plan was generated from a comprehensive architecture prompt covering 32 sections of engine extensibility design.
