# liara-core

> The core library for the Liara Engine.

This repository contains the c++ implementation of the Liara Engine's core
module. It is a consumer of the [Liara Interfaces](https://github.com/liara-engine/liara-interfaces).

This module provides the foundational components and abstractions for the Liara Engine, including memory management,
logging, configuration, and other essential utilities.

---

## Status

**Phase 0** — bootstrap. The core library is in the early stages of development, and the API is subject to change.

---

## Documentation

- **Architecture and rationale**: see the meta repository's
  [`docs/ARCHITECTURE.md`](https://liara-engine.liara-engine-documentation.workers.dev/liara/latest/book/ARCHITECTURE)
  and [`docs/MODULES.md`](https://liara-engine.liara-engine-documentation.workers.dev/liara/latest/book/MODULES).
- **Day-to-day workflow**: meta repository's
  [`docs/CONTRIBUTING.md`](https://liara-engine.liara-engine-documentation.workers.dev/liara/latest/book/CONTRIBUTING).
- **API reference** (Doxygen, deployed once Phase 0 completes):
  [liara-engine.liara-engine-documentation.workers.dev/liara-core](https://liara-engine.liara-engine-documentation.workers.dev/).

---

## Building Standalone

Most of the time you will not build this repository on its own; it is
consumed by other modules through `find_package(LiaraCore)` or
`add_subdirectory()`. To verify the headers compile and the tests pass
in isolation:

```bash
git clone https://github.com/liara-engine/liara-core.git
cd liara-core

cmake --preset=linux-debug-clang
cmake --build --preset=linux-debug-clang
ctest --preset=linux-debug-clang --output-on-failure
```

The full development setup (Vulkan SDK, vcpkg, presets, etc.) is
documented in the meta repository's
[`docs/BOOTSTRAP.md`](https://liara-engine.liara-engine-documentation.workers.dev/liara/latest/book/BOOTSTRAP).

---

## Consuming This Module

In a consumer's `CMakeLists.txt`:

```cmake
find_package(LiaraCore REQUIRED)
target_link_libraries(my_module PRIVATE Liara::Core)
```

---

## License

Released under the [MIT License](https://liara-engine.liara-engine-documentation.workers.dev/liara/latest/book/LICENSE).
