# ECS Core Files

This document describes the first three core files of the ECS framework.

## Current Files

```text
ECS/
└── Public/
    ├── Types.h
    ├── Entity.h
    └── IComponentPool.h
```

---

# 1. Types.h

## Purpose

`Types.h` contains the basic type definitions used throughout the ECS framework.

### Code

```cpp
#pragma once

#include <cstdint>
#include <cstddef>

namespace ECS
{
    using EntityID        = std::uint32_t;
    using Generation      = std::uint32_t;
    using ComponentTypeID = std::size_t;

    constexpr EntityID INVALID_ENTITY_ID = 0;
}
```

## Type Definitions

| Type              | Description                                       |
| ----------------- | ------------------------------------------------- |
| `EntityID`        | Unique numeric identifier for an entity           |
| `Generation`      | Version number used to validate recycled entities |
| `ComponentTypeID` | Unique identifier for a component type            |

### EntityID

```cpp
using EntityID = std::uint32_t;
```

Each entity receives an ID.

Example:

```text
Player  → EntityID 1
Enemy   → EntityID 2
Food    → EntityID 3
```

Entity ID `0` is reserved as invalid.

---

### Generation

```cpp
using Generation = std::uint32_t;
```

Generation helps prevent old entity references from becoming valid after an entity ID is reused.

Example:

```text
Entity { id = 5, generation = 1 }
```

After the entity is destroyed and ID `5` is reused:

```text
Entity { id = 5, generation = 2 }
```

The old entity reference is no longer valid.

---

### ComponentTypeID

```cpp
using ComponentTypeID = std::size_t;
```

This type will later be used to uniquely identify component types.

Example:

```text
Position → ComponentTypeID 0
Velocity → ComponentTypeID 1
Health   → ComponentTypeID 2
```

---

### INVALID_ENTITY_ID

```cpp
constexpr EntityID INVALID_ENTITY_ID = 0;
```

Entity ID `0` represents an invalid entity.

Example:

```cpp
if (entity.id == INVALID_ENTITY_ID)
{
    // Invalid entity
}
```

---

# 2. Entity.h

## Purpose

`Entity.h` defines the `Entity` structure.

An entity is a lightweight object containing:

* Entity ID
* Generation

### Code

```cpp
#pragma once

#include "Public/Types.h"

namespace ECS
{
    struct Entity
    {
        EntityID   id         = INVALID_ENTITY_ID;
        Generation generation = 0;

        bool operator==(const Entity& other) const
        {
            return id == other.id &&
                   generation == other.generation;
        }

        bool operator!=(const Entity& other) const
        {
            return !(*this == other);
        }

        bool IsValid() const
        {
            return id != INVALID_ENTITY_ID;
        }
    };

    constexpr Entity INVALID_ENTITY {};
}
```

---

## Entity Structure

An entity consists of two values:

```text
Entity
│
├── id
│
└── generation
```

Example:

```cpp
Entity player
{
    1,
    0
};
```

---

## id

```cpp
EntityID id;
```

The unique ID of the entity.

Example:

```text
Player → ID 1
Enemy  → ID 2
Food   → ID 3
```

---

## generation

```cpp
Generation generation;
```

The generation number identifies which version of an entity ID is being used.

Example:

```cpp
Entity entity
{
    10,
    3
};
```

This means:

```text
Entity ID:    10
Generation:   3
```

---

## operator==

```cpp
bool operator==(const Entity& other) const
```

Checks whether two entities are exactly the same.

Both values must match:

```text
ID         ✓
Generation ✓
```

Example:

```cpp
Entity a{1, 0};
Entity b{1, 0};

if (a == b)
{
    // Same entity
}
```

---

## operator!=

```cpp
bool operator!=(const Entity& other) const
```

Checks whether two entities are different.

Example:

```cpp
Entity a{1, 0};
Entity b{2, 0};

if (a != b)
{
    // Different entities
}
```

---

## IsValid()

```cpp
bool IsValid() const
```

Checks whether the entity contains a valid ID.

Example:

```cpp
Entity entity{};

if (!entity.IsValid())
{
    // Invalid entity
}
```

---

## INVALID_ENTITY

```cpp
constexpr Entity INVALID_ENTITY {};
```

Represents a complete invalid entity object.

It is equivalent to:

```cpp
constexpr Entity INVALID_ENTITY
{
    INVALID_ENTITY_ID,
    0
};
```

Example:

```cpp
Entity player = INVALID_ENTITY;
```

Check it:

```cpp
if (player == INVALID_ENTITY)
{
    // Player is invalid
}
```

---

# 3. IComponentPool.h

## Purpose

`IComponentPool` is the base interface for all component pools.

Every component pool will inherit from this class.

### Code

```cpp
#pragma once

#include "Public/Types.h"

namespace ECS
{
    class IComponentPool
    {
    public:
        virtual ~IComponentPool() = default;

        virtual void Remove(EntityID id) = 0;

        virtual bool Has(EntityID id) const = 0;
    };
}
```

---

# Component Pool Architecture

Future component pools will look like this:

```text
IComponentPool
      │
      │ Base Interface
      │
      ├─────────────────────────┐
      │                         │
      ▼                         ▼
ComponentPool<Position>   ComponentPool<Velocity>
      │                         │
      ▼                         ▼
Position Components       Velocity Components
```

Example:

```cpp
class ComponentPool<Position> : public IComponentPool
{
};
```

---

# Virtual Destructor

```cpp
virtual ~IComponentPool() = default;
```

This allows safe deletion through a base class pointer.

Example:

```cpp
IComponentPool* pool = new ComponentPool<Position>();

delete pool;
```

The correct `ComponentPool` destructor will be called.

---

# Remove()

```cpp
virtual void Remove(EntityID id) = 0;
```

Removes a component belonging to an entity.

Example:

```text
Entity ID 5
    │
    ▼
Position Component
    │
    ▼
Remove(5)
```

The component is removed from the pool.

---

# Has()

```cpp
virtual bool Has(EntityID id) const = 0;
```

Checks whether an entity has a component inside this pool.

Example:

```cpp
if (positionPool.Has(entity.id))
{
    // Entity has Position component
}
```

---

# Why Use EntityID Instead of Entity?

The component pool only needs the entity's ID to locate the component.

Example storage:

```text
Position Component Pool

EntityID        Position
──────────────────────────
1               {10, 20}
5               {30, 50}
8               {100, 200}
```

Therefore:

```cpp
Remove(EntityID id);
```

and:

```cpp
Has(EntityID id);
```

are appropriate.

---

# Current ECS Architecture

```text
ECS
│
├── Types.h
│   │
│   ├── EntityID
│   ├── Generation
│   ├── ComponentTypeID
│   └── INVALID_ENTITY_ID
│
├── Entity.h
│   │
│   ├── EntityID
│   ├── Generation
│   ├── IsValid()
│   ├── operator==
│   ├── operator!=
│   └── INVALID_ENTITY
│
└── IComponentPool.h
    │
    ├── Remove()
    └── Has()
```

---

# Current Progress

| File               | Status    |
| ------------------ | --------- |
| `Types.h`          | Completed |
| `Entity.h`         | Completed |
| `IComponentPool.h` | Completed |

---

# Next Step

The next recommended ECS file is:

```text
EntityManager.h
```

The `EntityManager` will handle:

* Creating entities
* Destroying entities
* Recycling entity IDs
* Increasing generations
* Checking whether entities are alive

## Development Order

```text
✓ Types.h
      ↓
✓ Entity.h
      ↓
✓ IComponentPool.h
      ↓
→ EntityManager.h
      ↓
ComponentType.h
      ↓
ComponentPool<T>
      ↓
ComponentManager
      ↓
Registry / World
```
