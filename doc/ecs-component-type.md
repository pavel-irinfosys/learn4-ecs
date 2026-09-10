# ComponentType.h

## Purpose

`ComponentType.h` generates a unique, stable `ComponentTypeID` for every
distinct component type used with the ECS.

It is responsible for:

* Assigning a unique numeric id to each component type (`Position`,
  `Velocity`, `Health`, etc.)
* Guaranteeing the same type always returns the same id
* Handing out ids sequentially, starting from `0`

### Code

```cpp
/**
 * ComponentType.h: generates a unique, stable ComponentTypeID for every
 * distinct component type used with the ECS.
 *
 * Usage:
 *     struct Position {};
 *     struct Velocity {};
 *
 *     ComponentTypeID posId = GetComponentTypeID<Position>();
 *     ComponentTypeID velId = GetComponentTypeID<Velocity>();
 *
 * Position and Velocity will each receive a distinct id (0, 1, 2, ...),
 * assigned in the order they are first requested. The same type always
 * returns the same id for the lifetime of the program.
 */

#pragma once

#include "Public/Types.h"

namespace ECS
{
    namespace Internal
    {
        /** Shared counter that hands out the next unused ComponentTypeID. */
        inline ComponentTypeID GetNextComponentTypeID()
        {
            static ComponentTypeID nextID = 0;
            return nextID++;
        }
    }

    /**
     * Returns the unique ComponentTypeID associated with type T.
     * The id is assigned the first time this is called for a given T,
     * and is stable for the remainder of the program's execution.
     */
    template<typename T>
    ComponentTypeID GetComponentTypeID()
    {
        static const ComponentTypeID id = Internal::GetNextComponentTypeID();
        return id;
    }
}
```

---

## Members

| Function                                | Location                | Description                                    |
| ---------------------------------------- | ------------------------ | ----------------------------------------------- |
| `GetComponentTypeID<T>()`                | `ECS`                    | Public API. Returns the id for type `T`.        |
| `Internal::GetNextComponentTypeID()`     | `ECS::Internal`          | Shared counter. Not meant to be called directly. |

`Internal` is a plain namespace, not an access-control mechanism -- it is a
naming convention that signals "implementation detail, don't call this
directly."

---

## GetComponentTypeID\<T\>()

```cpp
template<typename T>
ComponentTypeID GetComponentTypeID();
```

Returns the unique id associated with component type `T`.

### How it works

`GetComponentTypeID<T>()` is a function template. Each distinct `T` causes
the compiler to instantiate a separate copy of the function, and each copy
owns its own `static const ComponentTypeID id`. A local static is
initialized exactly once, the first time control passes through it -- so
the very first call for a given `T` assigns it the next available id, and
every later call for that same `T` just returns the already-initialized
value.

```text
GetComponentTypeID<Position>()
        │
        ▼
First call for Position? ──No──▶ return cached id
        │
       Yes
        │
        ▼
id = Internal::GetNextComponentTypeID()
        │
        ▼
cache id, return id
```

### Example

```cpp
struct Position {};
struct Velocity {};

ComponentTypeID posId = GetComponentTypeID<Position>(); // 0
ComponentTypeID velId = GetComponentTypeID<Velocity>(); // 1
ComponentTypeID again = GetComponentTypeID<Position>(); // 0 -- same as before
```

---

## Internal::GetNextComponentTypeID()

```cpp
inline ComponentTypeID GetNextComponentTypeID();
```

Hands out the next unused `ComponentTypeID` from a single shared counter.

```text
Before: nextID == 2
GetNextComponentTypeID()
After:  nextID == 3, returns 2
```

### Why not call this directly?

It only knows how to produce "the next number" -- it has no awareness of
which types already have ids. Calling it directly consumes an id that no
type owns, silently shifting every id assigned afterward.

```cpp
ComponentTypeID rogue = Internal::GetNextComponentTypeID(); // burns id 0

ComponentTypeID posId = GetComponentTypeID<Position>(); // now gets id 1, not 0
```

`GetComponentTypeID<T>()` is the only function meant to be called from
outside this file.

---

## Current Progress

| File                | Status    |
| ------------------- | --------- |
| `Types.h`           | Completed |
| `Entity.h`          | Completed |
| `IComponentPool.h`  | Completed |
| `EntityManager.h`   | Completed |
| `ComponentType.h`   | Completed |

---

## Next Step

```text
✓ Types.h
      ↓
✓ Entity.h
      ↓
✓ IComponentPool.h
      ↓
✓ EntityManager.h
      ↓
✓ ComponentType.h
      ↓
→ ComponentPool<T>
      ↓
ComponentManager
      ↓
Registry / World
```