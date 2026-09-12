# Registry.h

## Purpose

`Registry` is the main public ECS interface. It combines `EntityManager` and
`ComponentManager` behind a single, simple API, so calling code never has
to touch either one directly.

It is responsible for:

* Creating and destroying entities (delegating to `EntityManager`)
* Adding, removing, checking, and fetching components on entities
  (delegating to `ComponentManager`)
* Ensuring that destroying an entity also removes every component it
  owned, across every `ComponentPool` -- `EntityManager` alone only
  invalidates the `Entity` handle; it knows nothing about components
* Tracking which entities are currently alive, so that entity **queries**
  (`View<Components...>()`) have something to iterate over without going
  through `EntityManager` internals

`Registry` is fully header-only, consistent with `ComponentManager`.

### Code

```cpp
/**
 * Registry.h
 *
 * The main public ECS interface. Combines EntityManager and ComponentManager
 * behind a single, simple API:
 *
 *   Registry registry;
 *
 *   Entity player = registry.CreateEntity();
 *   registry.AddComponent(player, Position{10.0f, 20.0f});
 *   registry.AddComponent(player, Velocity{1.0f, 0.0f});
 *
 *   if (Position* pos = registry.GetComponent<Position>(player))
 *   {
 *       pos->x += 1.0f;
 *   }
 *
 *   registry.DestroyEntity(player);
 *
 * Responsibilities:
 *   - Create and destroy entities (delegating to EntityManager).
 *   - Add/Remove/Has/Get components on entities (delegating to
 *     ComponentManager).
 *   - Ensure that destroying an entity also removes every component it
 *     owned, across every ComponentPool -- EntityManager alone only
 *     invalidates the Entity handle, it knows nothing about components.
 *   - Track alive entities so View<Components...>() has something to
 *     iterate over.
 *
 * Internal structure:
 *
 *   Registry
 *       |
 *       +-- EntityManager     (entity lifetime, generations)
 *       +-- ComponentManager  (component storage, per type)
 *       +-- m_aliveEntities   (creation-order list of currently-alive entities)
 */

#pragma once

#include "Public/Types.h"
#include "Public/Entity.h"
#include "Public/EntityManager.h"
#include "Public/ComponentManager.h"

#include <vector>
#include <algorithm>

namespace ECS
{
    class Registry
    {
    public:
        Registry() = default;

        /** Creates a new entity. Delegates to EntityManager, and tracks it as alive. */
        Entity CreateEntity()
        {
            Entity entity = m_entityManager.Create();
            m_aliveEntities.push_back(entity);
            return entity;
        }

        /**
         * Destroys an entity: removes every component it owned across all
         * pools, then invalidates its handle and recycles its EntityID.
         * A no-op if the entity is already stale or invalid.
         */
        void DestroyEntity(Entity entity)
        {
            if (!m_entityManager.IsAlive(entity))
            {
                return;
            }

            m_componentManager.RemoveEntityComponents(entity.id);
            m_entityManager.Destroy(entity);

            auto it = std::find(m_aliveEntities.begin(), m_aliveEntities.end(), entity);
            if (it != m_aliveEntities.end())
            {
                m_aliveEntities.erase(it);
            }
        }

        /** Returns true if the entity handle still refers to a currently-live entity. */
        bool IsAlive(Entity entity) const
        {
            return m_entityManager.IsAlive(entity);
        }

        /** Adds (or overwrites) a component of type T on the given entity. */
        template<typename T>
        void AddComponent(Entity entity, const T& component)
        {
            m_componentManager.AddComponent<T>(entity, component);
        }

        /** Removes the component of type T from the given entity, if present. */
        template<typename T>
        void RemoveComponent(Entity entity)
        {
            m_componentManager.RemoveComponent<T>(entity);
        }

        /** Returns true if the given entity has a component of type T. */
        template<typename T>
        bool HasComponent(Entity entity) const
        {
            return m_componentManager.HasComponent<T>(entity);
        }

        /** Returns a mutable pointer to the entity's component of type T, or nullptr if absent. */
        template<typename T>
        T* GetComponent(Entity entity)
        {
            return m_componentManager.GetComponent<T>(entity);
        }

        /** Returns a const pointer to the entity's component of type T, or nullptr if absent. */
        template<typename T>
        const T* GetComponent(Entity entity) const
        {
            return m_componentManager.GetComponent<T>(entity);
        }

        /**
         * Returns every currently-alive entity that has ALL of the given
         * component types.
         *
         * First implementation: walks every alive entity and checks
         * HasComponent<T>() for each requested type. O(aliveEntities *
         * componentTypes) per call -- simple and correct, not yet
         * optimized. A future version can back this with a sparse-set
         * intersection instead of a linear scan.
         *
         * Example:
         *   for (Entity entity : registry.View<Position, Velocity>())
         *   {
         *       auto* pos = registry.GetComponent<Position>(entity);
         *       auto* vel = registry.GetComponent<Velocity>(entity);
         *       pos->x += vel->x;
         *       pos->y += vel->y;
         *   }
         */
        template<typename... Components>
        std::vector<Entity> View() const
        {
            std::vector<Entity> result;

            for (const Entity& entity : m_aliveEntities)
            {
                if ((HasComponent<Components>(entity) && ...))
                {
                    result.push_back(entity);
                }
            }

            return result;
        }

    private:
        EntityManager      m_entityManager;
        ComponentManager   m_componentManager;
        std::vector<Entity> m_aliveEntities; ///< Every entity currently alive, in creation order.
    };
}
```

---

## Members

| Member             | Type                 | Description                                              |
| -------------------- | -------------------- | ---------------------------------------------------------- |
| `m_entityManager`    | `EntityManager`      | Owns entity lifetime, IDs, and generations.               |
| `m_componentManager` | `ComponentManager`   | Owns component storage, one pool per type.                |
| `m_aliveEntities`    | `std::vector<Entity>` | Every entity currently alive, in creation order.          |

```text
Registry
    |
    +-- EntityManager     (entity lifetime, generations)
    +-- ComponentManager  (component storage, per type)
    +-- m_aliveEntities   (creation-order list, backs View<Components...>())
```

`Registry` holds no entity-lifetime or component-storage logic of its own
-- both `EntityManager` and `ComponentManager` do the real work, and
`Registry` is purely the glue that keeps them in sync with each other.
`m_aliveEntities` is the one piece of state `Registry` owns directly: it
exists solely so `View<Components...>()` has a list of live entities to
scan, without reaching into `EntityManager`'s internal generation/free-list
storage.

---

## CreateEntity()

```cpp
Entity CreateEntity();
```

Creates a new entity: delegates to `EntityManager::Create()`, then appends
the returned handle to `m_aliveEntities`.

```text
CreateEntity()
        |
        v
m_entityManager.Create()
        |
        v
m_aliveEntities.push_back(entity)
        |
        v
return entity
```

### Example

```cpp
Registry registry;
Entity player = registry.CreateEntity();
```

---

## DestroyEntity()

```cpp
void DestroyEntity(Entity entity);
```

Destroys an entity: removes every component it owned across all pools,
invalidates its handle and recycles its `EntityID`, then removes it from
`m_aliveEntities`. A no-op if the entity is already stale or invalid.

```text
DestroyEntity(entity)
        |
        v
IsAlive(entity)? --No--> return (no-op)
        |
       Yes
        |
        v
m_componentManager.RemoveEntityComponents(entity.id)
        |
        v
m_entityManager.Destroy(entity)
        |
        v
find entity in m_aliveEntities --> erase it
```

### Why check IsAlive() first?

The order matters for two reasons:

* **Stale/double-destroy safety.** If `DestroyEntity` is called twice on
  the same entity, or on an already-recycled `EntityID` that now belongs
  to a different, newer entity, the `IsAlive()` check stops it from
  removing components -- or the alive-list entry -- that belong to that
  newer entity.
* **Components must be removed before the ID is recycled.** Removing
  components first, then destroying the handle, guarantees a freshly
  recycled `EntityID` never inherits leftover components from whoever
  used that ID before.

### Why erase from m_aliveEntities last?

`m_aliveEntities` is only used for queries (`View<Components...>()`), so it
is kept in sync last, after the entity and component state has already
been safely torn down. A linear `std::find` + `erase` is O(n) -- fine for a
first implementation, and easy to replace later with a swap-and-pop or an
index map if this becomes a hot path.

### Example

```cpp
registry.DestroyEntity(player);
// player is now stale; all its components have been removed too,
// and it no longer appears in registry.View<...>() results.

registry.DestroyEntity(player); // no-op -- already destroyed
```

---

## IsAlive()

```cpp
bool IsAlive(Entity entity) const;
```

Returns true if the entity handle still refers to a currently-live entity.
A direct pass-through to `EntityManager::IsAlive()`.

### Example

```cpp
if (registry.IsAlive(player))
{
    // player has not been destroyed
}
```

---

## AddComponent\<T\>()

```cpp
template<typename T>
void AddComponent(Entity entity, const T& component);
```

Adds (or overwrites) a component of type `T` on the given entity. A direct
pass-through to `ComponentManager::AddComponent<T>()`.

### Example

```cpp
registry.AddComponent(player, Position{10.0f, 20.0f});
registry.AddComponent(player, Velocity{1.0f, 0.0f});
```

---

## RemoveComponent\<T\>()

```cpp
template<typename T>
void RemoveComponent(Entity entity);
```

Removes the component of type `T` from the given entity, if present. A
direct pass-through to `ComponentManager::RemoveComponent<T>()`.

### Example

```cpp
registry.RemoveComponent<Velocity>(player);
```

---

## HasComponent\<T\>()

```cpp
template<typename T>
bool HasComponent(Entity entity) const;
```

Returns true if the given entity currently has a component of type `T`. A
direct pass-through to `ComponentManager::HasComponent<T>()`. Also the
building block `View<Components...>()` calls once per requested type.

### Example

```cpp
if (registry.HasComponent<Position>(player))
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
the entity has no component of that type. A direct pass-through to
`ComponentManager::GetComponent<T>()`.

### Example

```cpp
if (Position* pos = registry.GetComponent<Position>(player))
{
    pos->x += 1.0f;
}
```

---

## View\<Components...\>()

```cpp
template<typename... Components>
std::vector<Entity> View() const;
```

Returns every currently-alive entity that has **all** of the given
component types.

```text
View<Position, Velocity>()
        |
        v
result = []
        |
        v
for entity in m_aliveEntities:
        HasComponent<Position>(entity)
            AND HasComponent<Velocity>(entity)
        |
       Yes --> result.push_back(entity)
        |
        No --> skip
        |
        v
return result
```

### How it works

`Components...` is a template parameter pack -- one type per requested
component. The check

```cpp
(HasComponent<Components>(entity) && ...)
```

is a C++17 fold expression: it expands to
`HasComponent<C1>(entity) && HasComponent<C2>(entity) && ... && HasComponent<Cn>(entity)`
for whatever types were passed to `View`, short-circuiting on the first
`false` just like a hand-written chain of `&&` would.

### Cost

This is a **linear scan**, not an index-backed query: every alive entity
is checked against every requested component type, so a call costs
`O(aliveEntities * sizeof...(Components))`. That's simple and correct, but
not the fastest possible approach -- a future version can back `View` with
a sparse-set intersection (finding the smallest relevant pool and only
checking membership in the others) instead of walking every alive entity
per call.

### Example

```cpp
for (Entity entity : registry.View<Position, Velocity>())
{
    auto* pos = registry.GetComponent<Position>(entity);
    auto* vel = registry.GetComponent<Velocity>(entity);

    pos->x += vel->x;
    pos->y += vel->y;
}
```

Note that `GetComponent` is still called separately inside the loop --
`View` only tells you *which* entities qualify, it does not hand back the
components themselves.

---

## Full Usage Example

```cpp
ECS::Registry registry;
ECS::Entity player = registry.CreateEntity();

registry.AddComponent(player, Position{10.0f, 20.0f});
registry.AddComponent(player, Velocity{1.0f, 0.0f});

if (auto* pos = registry.GetComponent<Position>(player))
{
    pos->x += 1.0f;
}

for (ECS::Entity entity : registry.View<Position, Velocity>())
{
    auto* pos = registry.GetComponent<Position>(entity);
    auto* vel = registry.GetComponent<Velocity>(entity);
    pos->x += vel->x;
    pos->y += vel->y;
}

registry.DestroyEntity(player);
```

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
| `Registry.h`         | Completed (now includes `View<Components...>()`) |

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
✓ Registry.h (incl. View<Components...>())
      ↓
→ Systems (System.h)
      ↓
SystemManager
      ↓
Performance: sparse-set-backed View
```
