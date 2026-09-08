# EntityManager.h

## Purpose

`EntityManager` owns the lifetime of every `Entity` in the ECS.

It is responsible for:

* Creating entities
* Destroying entities
* Recycling entity IDs
* Increasing generations
* Checking whether entities are alive

### Code

```cpp
/**
 * EntityManager: owns the lifetime of every Entity in the ECS.
 *
 * Responsibilities:
 *   - Create()   allocates a new Entity, reusing a recycled Id when possible.
 *   - Destroy()  invalidates an Entity by bumping its generation, then
 *                recycles the Id for future reuse.
 *   - IsAlive()  checks whether a given Entity handle still matches the
 *                current generation stored for its Id.
 *
 * Ids are never freed from memory; they are recycled. Each Id has an
 * associated generation counter that increases every time the Id is
 * destroyed and reused, which is what allows stale Entity handles to be
 * detected after their Id has been reassigned to a new entity.
 */

#pragma once

#include "Public/Types.h"
#include "Public/Entity.h"

#include <vector>

namespace ECS
{
    class EntityManager
    {
    public:
        EntityManager() = default;

        /** Allocates a new Entity, recycling a freed Id if one is available. */
        Entity Create();

        /** Invalidates an Entity by bumping its generation and recycling its Id. */
        void Destroy(Entity entity);

        /** Returns true if the Entity's Id and generation match the currently live entity. */
        bool IsAlive(Entity entity) const;

    private:
        std::vector<Generation> m_generations; ///< Current generation for each EntityID.
        std::vector<EntityID>   m_freeIDs;      ///< Pool of destroyed Ids ready for reuse.
    };
}
```

---

## Members

| Member          | Type                       | Description                                    |
| --------------- | -------------------------- | ----------------------------------------------- |
| `m_generations`  | `std::vector<Generation>` | Current generation for each `EntityID`.         |
| `m_freeIDs`      | `std::vector<EntityID>`   | Pool of destroyed Ids that are ready for reuse. |

`m_generations` is indexed directly by `EntityID`. It never shrinks -- slots
are reused, never removed.

---

## Create()

```cpp
Entity Create();
```

Allocates a new entity, recycling a freed `EntityID` when one is available.

### Case 1: A free Id exists

```text
m_freeIDs: [5]
        │
        ▼
   pop back → id = 5
        │
        ▼
Entity{ 5, m_generations[5] }
```

The recycled Id keeps whatever generation it was left at by `Destroy()`, so
the returned handle is automatically valid and distinguishable from any
older, stale handle for the same Id.

### Case 2: No free Id exists

```text
m_freeIDs: []
        │
        ▼
new id = m_generations.size()
        │
        ▼
m_generations.push_back(0)
        │
        ▼
Entity{ id, 0 }
```

A brand-new slot is appended with generation `0`.

### Example

```cpp
EntityManager manager;

Entity player = manager.Create(); // Entity{ 0, 0 }
Entity enemy  = manager.Create(); // Entity{ 1, 0 }
```

---

## Destroy()

```cpp
void Destroy(Entity entity);
```

Invalidates an entity and recycles its `EntityID`.

```text
Destroy(entity)
      │
      ▼
IsAlive(entity)? ──No──▶ return (no-op)
      │
     Yes
      │
      ▼
++m_generations[entity.id]
      │
      ▼
m_freeIDs.push_back(entity.id)
```

### Why check IsAlive() first?

Without the check, calling `Destroy()` twice on the same entity -- or on a
stale handle -- would push the same `EntityID` onto `m_freeIDs` more than
once, corrupting the free list.

### What actually changes

Nothing is erased. The Id's generation is incremented, which is what makes
any existing `Entity` handle referencing the old generation invalid:

```text
Before: m_generations[5] == 2
Destroy(Entity{5, 2})
After:  m_generations[5] == 3
```

### Example

```cpp
Entity player = manager.Create(); // Entity{ 0, 0 }

manager.Destroy(player);
// player is now stale -- IsAlive(player) == false

Entity next = manager.Create();
// next == Entity{ 0, 1 } -- same Id, new generation
```

### Note

`Destroy()` only manages the `Entity` handle itself. It does not remove any
components. Clearing an entity's components from each `ComponentPool<T>` is
a separate step, expected to be handled by the future `Registry` / `World`.

---

## IsAlive()

```cpp
bool IsAlive(Entity entity) const;
```

Checks whether an `Entity` handle still refers to a currently-live entity.

```text
IsAlive(entity)
      │
      ▼
entity.IsValid()? ──No──▶ false
      │
     Yes
      │
      ▼
entity.id < m_generations.size()? ──No──▶ false
      │
     Yes
      │
      ▼
m_generations[entity.id] == entity.generation
```

### Example

```cpp
Entity a = manager.Create();      // Entity{ 0, 0 }

manager.IsAlive(a);               // true

manager.Destroy(a);

manager.IsAlive(a);                // false -- generation mismatch

Entity b = manager.Create();       // Entity{ 0, 1 } -- same Id as a
manager.IsAlive(b);                // true
manager.IsAlive(a);                // still false
```

### Relationship to Entity::IsValid()

| Check                     | Needs registry access? | Detects stale handles? |
| -------------------------- | :---------------------: | :----------------------: |
| `Entity::IsValid()`        | No                      | No                       |
| `EntityManager::IsAlive()` | Yes                     | Yes                      |

`IsValid()` only checks that `id != INVALID_ENTITY_ID` -- it is a cheap,
local, structural check. `IsAlive()` is the real liveness check: it
compares the handle's generation against the one currently stored for its
Id.

---

## Current Progress

| File               | Status    |
| ------------------ | --------- |
| `Types.h`          | Completed |
| `Entity.h`         | Completed |
| `IComponentPool.h` | Completed |
| `EntityManager.h`  | Completed |

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
→ ComponentType.h
      ↓
ComponentPool<T>
      ↓
ComponentManager
      ↓
Registry / World
```