# ComponentManager.h

## Purpose

`ComponentManager` owns one `ComponentPool<T>` per component type that has
ever been used, and routes `Add`/`Remove`/`Has`/`Get` calls to the correct
pool based on `T`.

It is responsible for:

* Lazily creating a `ComponentPool<T>` the first time type `T` is added
* Storing all pools behind `IComponentPool*`, keyed by `ComponentTypeID`
* Adding, removing, checking, and fetching components for a given `Entity`
  and type `T`
* Removing every component belonging to an entity, across all pools, when
  that entity is destroyed

`ComponentManager` is fully header-only -- every member function, template
or not, is defined inline inside the class. There is no accompanying
`ComponentManager.cpp`.

### Code

```cpp
/**
 * ComponentManager.h
 *
 * Owns one ComponentPool<T> per component type that has ever been used,
 * and routes Add/Remove/Has/Get calls to the correct pool based on T.
 */

#pragma once

#include "Public/Types.h"
#include "Public/Entity.h"
#include "Public/IComponentPool.h"
#include "Public/ComponentPool.h"
#include "Public/ComponentType.h"

#include <unordered_map>
#include <memory>

namespace ECS
{
    class ComponentManager
    {
    public:
        ComponentManager() = default;

        template<typename T>
        void AddComponent(Entity entity, const T& component);

        template<typename T>
        void RemoveComponent(Entity entity);

        template<typename T>
        bool HasComponent(Entity entity) const;

        template<typename T>
        T* GetComponent(Entity entity);

        template<typename T>
        const T* GetComponent(Entity entity) const;

        void RemoveEntityComponents(EntityID id);

    private:
        template<typename T>
        ComponentPool<T>& GetOrCreatePool();

        template<typename T>
        IComponentPool* FindPool();

        template<typename T>
        const IComponentPool* FindPool() const;

        std::unordered_map<ComponentTypeID, std::unique_ptr<IComponentPool>> m_pools;
    };
}
```

All member functions above -- including `RemoveEntityComponents`, which is
not a template -- are defined directly inside the class body. A member
function defined inside its class is implicitly `inline`, so this header
can be safely included from multiple `.cpp` files without any linker
errors, even for the non-template method.

---

## Members

| Member    | Type                                                                    | Description                                      |
| --------- | ------------------------------------------------------------------------ | -------------------------------------------------- |
| `m_pools` | `std::unordered_map<ComponentTypeID, std::unique_ptr<IComponentPool>>` | One pool per component type that has ever been used. |

```text
ComponentManager
        |
        v
ComponentTypeID -> IComponentPool*
        |
        +-- ComponentPool<Position>
        +-- ComponentPool<Velocity>
        +-- ComponentPool<Health>
```

`m_pools` is created lazily -- a type only gets an entry the first time
`AddComponent<T>()` is called for it. `GetComponent`/`RemoveComponent`/
`HasComponent` never create an entry; they only look one up.

---

## AddComponent\<T\>()

```cpp
template<typename T>
void AddComponent(Entity entity, const T& component);
```

Adds (or overwrites) a component of type `T` on the given entity.

```text
AddComponent<Position>(entity, component)
        |
        v
GetOrCreatePool<Position>()
        |
        v
pool.Add(entity.id, component)
```

If no `ComponentPool<Position>` exists yet, one is created and stored in
`m_pools` before the component is added.

### Example

```cpp
ComponentManager components;
Entity player{ 0, 0 };

components.AddComponent(player, Position{ 10.0f, 20.0f });
```

---

## RemoveComponent\<T\>()

```cpp
template<typename T>
void RemoveComponent(Entity entity);
```

Removes the component of type `T` from the given entity, if present.

```text
RemoveComponent<Position>(entity)
        |
        v
FindPool<Position>() exists? --No--> no-op
        |
       Yes
        |
        v
pool->Remove(entity.id)
```

Unlike `AddComponent`, this never creates a pool -- if type `T` has never
been used, there is nothing to remove, and the call is a no-op.

### Example

```cpp
components.RemoveComponent<Position>(player);
```

---

## HasComponent\<T\>()

```cpp
template<typename T>
bool HasComponent(Entity entity) const;
```

Returns true if the given entity currently has a component of type `T`.

```text
HasComponent<Position>(entity)
        |
        v
FindPool<Position>() exists? --No--> false
        |
       Yes
        |
        v
pool->Has(entity.id)
```

### Example

```cpp
if (components.HasComponent<Position>(player))
{
    // player has a Position component
}
```

---

## GetComponent\<T\>()

```cpp
template<typename T>
T* GetComponent(Entity entity);

template<typename T>
const T* GetComponent(Entity entity) const;
```

Returns a pointer to the entity's component of type `T`, or `nullptr` if
the entity has no component of that type (or no pool for `T` has ever been
created). **Never creates a pool** and **never asserts** -- a missing
component is a normal, checkable outcome, not a programmer error.

```text
GetComponent<Position>(entity)
        |
        v
FindPool<Position>() == nullptr? --Yes--> nullptr
        |
        No
        |
        v
pool->Get(entity.id)  -- nullptr if entity.id not in pool
```

Both the mutable and `const` overloads exist so that `GetComponent` can be
called correctly whether you have a `ComponentManager&` or a
`const ComponentManager&` -- the compiler picks whichever matches.

### Example

```cpp
if (Position* pos = components.GetComponent<Position>(player))
{
    pos->x += 1.0f;
}
else
{
    // player has no Position component
}
```

### Why a pointer instead of a reference?

An earlier version returned `T&` and used `assert()` to guard against a
missing component. That has two problems: `assert()` is compiled out
entirely in release builds (so a missing component becomes undefined
behavior instead of a safe check), and a reference can never represent
"nothing" the way a pointer can with `nullptr`. Returning `T*` makes a
missing component a normal, checkable result in every build configuration.

---

## RemoveEntityComponents()

```cpp
void RemoveEntityComponents(EntityID id)
{
    for (const auto& [typeID, pool] : m_pools)
    {
        pool->Remove(id);
    }
}
```

Removes every component belonging to the given `EntityID`, across every
pool. Intended to be called when an entity is destroyed, before its
`EntityID` is recycled -- this is the piece of cleanup `EntityManager`
itself does not do (see `EntityManager.h`'s note that destroying an entity
handle does not remove its components).

Not a template, but defined inline in the class body anyway -- a member
function defined inside its class is implicitly `inline`, so this is safe
to include from multiple `.cpp` files with no linker errors, and it keeps
the whole class in one file.

```text
RemoveEntityComponents(id)
        |
        v
for each (typeID, pool) in m_pools:
        pool->Remove(id)
```

Every pool's `Remove()` is safe to call even if that entity never had a
component of that type -- `ComponentPool<T>::Remove()` is a no-op on a
missing id.

### Example

```cpp
components.RemoveEntityComponents(player.id);
// player's Position, Velocity, Health, etc. -- all removed, whichever it had
```

---

## GetOrCreatePool\<T\>() (private)

```cpp
template<typename T>
ComponentPool<T>& GetOrCreatePool();
```

Returns the pool for `T`, creating it the first time `T` is used.

```text
GetOrCreatePool<Position>()
        |
        v
m_pools contains ComponentTypeID for Position? --No--> create & store ComponentPool<Position>
        |
       Yes
        |
        v
return existing pool
```

Only called from `AddComponent` -- the only operation that should ever
bring a new pool into existence.

---

## FindPool\<T\>() (private)

```cpp
template<typename T>
IComponentPool* FindPool();

template<typename T>
const IComponentPool* FindPool() const;
```

Returns the pool for `T`, or `nullptr` if `T` has never been used.

```text
FindPool<Position>()
        |
        v
m_pools contains ComponentTypeID for Position? --No--> nullptr
        |
       Yes
        |
        v
return pool
```

Used by `RemoveComponent`, `HasComponent`, and `GetComponent` -- all
read-or-modify-existing operations that should never spring a new, empty
pool into existence just by being called.

---

## Current Progress

| File                 | Status    |
| --------------------- | --------- |
| `Types.h`            | Completed |
| `Entity.h`           | Completed |
| `IComponentPool.h`   | Completed |
| `EntityManager.h`    | Completed |
| `ComponentType.h`    | Completed |
| `ComponentPool.h`    | Completed |
| `ComponentManager.h` | Completed |

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
✓ ComponentPool.h
      ↓
✓ ComponentManager.h
      ↓
→ Registry / World
```