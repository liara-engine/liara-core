---
title: liara-core
description: What the core owns, the test that decides what belongs in it, and what exists today as opposed to what is planned.
sidebar:
  label: Overview
  order: 0
---

The core implements everything that is shared, mandatory and not replaceable. One question decides what goes in it: what is always there, whatever modules are loaded?

It owns the data and the schedule, and the other modules transform data on a schedule it dictates. It is still a sibling of theirs rather than a runtime sitting above them: it obeys the same rules, carries one ABI namespace, exports one `liara_core_info()`, versions on its own cadence, and reaches for no other module by name.

## What it owns

**The ECS.** Entity allocation with generational handles, sparse-set component storage, the world container, the query API, system scheduling. Written by hand, for the reasons in [ADR 0009](https://liara-engine.liara-engine-documentation.workers.dev/liara/latest/guides/adr/0009-in-house-ecs/).

**The maths layer.** Vectors, matrices and quaternions as plain C structs declared in `liara-interfaces`, with the implementation functions here. GLM may be used internally, and no GLM type crosses the boundary.

**The logger.** Structured entries, several sinks, runtime level control, thread safety.

**The settings system.** Type-safe key-value storage serialized to TOML, with change notifications.

**The event bus.** Publish-subscribe for engine events and for input events arriving from the platform module through the host.

**The loop primitives.** `liara_core_update(core, dt)` advances the simulation by one tick, and `liara_core_get_render_packet()` hands back what the host submits to the renderer.

## What it does not

No rendering. No window, no input device, no OS signal, which are `liara-platform`. No file loading or decoding, which is `liara-assets`. No audio device, no editor code, no gameplay.

The practical test is narrower than any list: the core does not open a file, does not talk to a device, and calls no OS API beyond threading and time. If it needs the outside world, it is not core.

It also never creates, loads or references another module. Pairing it with a renderer, checking their versions against each other and wiring them together is the [host's job](https://liara-engine.liara-engine-documentation.workers.dev/liara/latest/guides/adr/0003-the-host-composes-modules/).

## What exists today

A placeholder. The core can be created, ticked, told which mesh to draw, asked for a render packet, and destroyed, which is enough to prove module composition works end to end. None of the subsystems above is written: `src/` holds one file, and `vcpkg.json` declares no runtime dependency at all.

The packet it builds is v0.1's experimental one: a single view on the window, cleared to dark blue, which draws one mesh once the host has named it through `liara_core_draw_mesh()`. That entry point is experimental too. It stands in for what v0.2 turns into creating an entity with a mesh component, and it goes with the packet in ABI 2.0.0. Nothing simulates, so a tick only rebuilds the packet.

The ECS and the maths types arrive with v0.2, and the [roadmap](https://liara-engine.liara-engine-documentation.workers.dev/liara/latest/guides/roadmap/v0-2/) says what that milestone contains.

## Where to go next

The [API reference](https://liara-engine.liara-engine-documentation.workers.dev/liara-core/latest/api/) is generated from the headers in `liara-interfaces`, which is where this module's contract lives. The internal C++ behind it is private and deliberately undocumented here, since no other module ever sees it.
