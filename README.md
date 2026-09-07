# SDL3 ECS

A small Entity Component System (ECS) built from scratch in C++20, paired with SDL3 for windowing and rendering.

## Project structure

```
sdl3-ecs/
├── CMakeLists.txt
└── src/
    ├── main.cpp
    ├── ECS/
    │   ├── Public/      (Types.h, Entity.h, IComponentPool.h, ComponentPool.h, Registry.h, System.h)
    │   └── Private/      (Registry.cpp)
    ├── Components/
    │   └── Public/        (TransformComponent.h, VelocityComponent.h)
    └── Systems/
        ├── Public/
        └── Private/
```

## Requirements

- CMake 3.20+
- A C++20 compiler (GCC, Clang, or MSVC)
- SDL3 installed and discoverable by CMake (`find_package(SDL3 REQUIRED CONFIG)`)

## Build

From the project root:

```bash
cmake -B build -S .
cmake --build build
```

## Run

```bash
./build/SDL3ECS
```

## Rebuilding after changes

You don't need to delete `build/` for normal source changes — just re-run:

```bash
cmake --build build
```

Only do a clean reconfigure if CMake itself is misbehaving (e.g. after editing `CMakeLists.txt` significantly, or changing SDL3 install paths):

```bash
rm -rf build
cmake -B build -S .
cmake --build build
```

## Troubleshooting

- **`CMake Error: ... does not appear to contain CMakeLists.txt`** — make sure `CMakeLists.txt` is spelled exactly that way (case-sensitive on Linux/macOS) and sits in the project root, and that you're running `cmake` from inside that root directory.
- **SDL3 not found** — confirm it's installed and visible to CMake:
  ```bash
  pkg-config --modversion sdl3
  ```
  If that fails, SDL3 isn't installed/discoverable yet — install it via your package manager, vcpkg, or from source before reconfiguring.