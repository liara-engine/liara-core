---
title: About Liara Core
description: The core module of the Liara Engine.
sidebar:
    order: 0
---

# liara-core

> The core module of the Liara Engine.

The C++ implementation of everything that is shared, mandatory and not replaceable: the ECS, the maths types, the logger, the settings, the event bus and the loop primitives. It answers one question, which is what is always there whatever else is loaded.

It owns the data model the rest of the engine agrees on, and it is still a sibling of the other modules rather than a runtime above them. It reaches for none of them by name, and its only dependency is [liara-interfaces](https://github.com/liara-engine/liara-interfaces).

## Status

**Phase 0.** What exists is a placeholder implementation sufficient to prove the module composition works end to end: a core that can be created, ticked, and asked for a render packet. The ECS arrives with v0.2, and none of the subsystems listed above is written yet.

## Documentation

- **[Module guide](https://liara-engine.liara-engine-documentation.workers.dev/liara-core/latest/guides/)**: what this module is for and what it will not contain.
- **[API reference](https://liara-engine.liara-engine-documentation.workers.dev/liara-core/latest/api/)**: generated from the headers.
- **Architecture**: the meta repository's [architecture](https://liara-engine.liara-engine-documentation.workers.dev/liara/latest/guides/architecture/) and [modules](https://liara-engine.liara-engine-documentation.workers.dev/liara/latest/guides/modules/core/) pages.
- **Workflow**: [contributing](https://liara-engine.liara-engine-documentation.workers.dev/liara/latest/guides/contributing/).

## Building it on its own

Usually you do not. This repository is consumed through the workspace superbuild, where `liara-interfaces` is built alongside it, or through `find_package(LiaraCore)` from an install.

Two things make a bare clone awkward, and both are deliberate. It has no `CMakePresets.json`, because presets are generated into the workspace from a template in the meta repository. And it fails to configure without `Liara::Interfaces`, with a message saying so, since a module built against no contract is a module built against nothing.

To build it alone against an installed contract:

```bash
git clone https://github.com/liara-engine/liara-core.git
cd liara-core

cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_PREFIX_PATH=/path/to/liara-interfaces/install
cmake --build build
ctest --test-dir build --output-on-failure
```

The normal setup, which is one command, is in [bootstrap](https://liara-engine.liara-engine-documentation.workers.dev/liara/latest/guides/bootstrap/).

## Consuming it

```cmake
find_package(LiaraCore REQUIRED)
target_link_libraries(my_module PRIVATE Liara::Core)
```

Always through the `Liara::Core` alias rather than the bare target name, so that a build compiles against exactly what an external consumer gets.

## License

[MIT](https://liara-engine.liara-engine-documentation.workers.dev/liara/latest/about/license/).
