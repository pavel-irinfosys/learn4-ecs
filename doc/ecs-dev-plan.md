# ECS Framework Development Plan

## Project Goal

Build a lightweight, reusable **Entity Component System (ECS)** framework in C++.

The framework should support:

* Entity creation and destruction
* Entity generations
* Component registration
* Adding and removing components
* Component pools
* Fast component lookup
* Entity queries
* Systems
* Future embedded/ESP32 compatibility

---

# 1. Core Types

**Status: Completed**

File:

```text
Public/Types.h
```

Current types:

```cpp
namespace ECS
{
    using EntityID        = std::uint32_t;
    using Generateion     = std::uint32_t;
    using ComponentTypeID = std::size_t;

    constexpr EntityID INVALID_ENTITY_ID = 0;
}
```

### Purpose

| Type              | Purpose                                         |
| ----------------- | ----------------------------------------------- |
| `EntityID`        | Unique entity index                             |
| `Generation`      | Prevent invalid references to recycled entities |
| `ComponentTypeID` | Unique identifier for each component type       |

### TODO

Fix the spelling:

```cpp
Generateion
```

Should be:

```cpp
Generation
```

Recommended version:

```cpp
using Generation = std::uint32_t;
```

---

# 2. Entity

**Status: Completed**

File:

```text
Entity.h
```

Current responsibilities:

* Store entity ID
* Store generation
* Compare entities
* Check validity

Structure:

```cpp
struct Entity
{
    EntityID id;
    Generation generation;

    bool operator==(const Entity& other) const;

    bool IsValid() const;
};
```

### Future Improvements

Add:

```cpp
bool operator!=(const Entity& other) const;
```

Optional invalid entity helper:

```cpp
constexpr Entity INVALID_ENTITY{};
```

---

# 3. Entity Manager

**Status: Next Step**

File:

```text
EntityManager.h
```

The Entity Manager will be responsible for:

* Creating entities
* Destroying entities
* Recycling entity IDs
* Managing generations
* Checking entity validity

### Internal Data

```text
EntityManager
│
├── generations[]
│
├── freeEntityIDs[]
│
└── aliveEntityCount
```

### Main Functions

```cpp
Entity CreateEntity();

void DestroyEntity(Entity entity);

bool IsAlive(Entity entity) const;

std::size_t GetEntityCount() const;
```

### Entity Creation Logic

```text
CreateEntity()
      │
      ▼
Is free ID available?
   │         │
 YES        NO
   │         │
Reuse ID   Create new ID
   │         │
   └────┬────┘
        ▼
Return Entity
```

### Entity Destruction Logic

```text
DestroyEntity(Entity)
        │
        ▼
Validate Entity
        │
        ▼
Increase Generation
        │
        ▼
Return ID to Free List
```

---

# 4. Component Type Registry

**Status: Planned**

File:

```text
ComponentType.h
```

Purpose:

Generate a unique ID for every component type.

Example:

```cpp
struct Position {};
struct Velocity {};
```

Each component receives:

```text
Position → ComponentTypeID 0
Velocity → ComponentTypeID 1
```

Expected API:

```cpp
template<typename T>
ComponentTypeID GetComponentTypeID();
```

---

# 5. Component Pool Interface

**Status: Completed**

File:

```text
IComponentPool.h
```

Current interface:

```cpp
class IComponentPool
{
public:
    virtual ~IComponentPool() = default;

    virtual void Remove(EntityID id) = 0;

    virtual bool Has(EntityID id) const = 0;
};
```

### Purpose

Allow different component pools to be stored through a common interface.

Example:

```text
IComponentPool
      │
      ├── ComponentPool<Position>
      │
      ├── ComponentPool<Velocity>
      │
      └── ComponentPool<Health>
```

---

# 6. Component Pool

**Status: Next Step**

File:

```text
ComponentPool.h
```

This will store components of one type.

Example:

```text
ComponentPool<Position>
```

### Required Functions

```cpp
template<typename T>
class ComponentPool : public IComponentPool
{
public:

    void Add(EntityID id, const T& component);

    void Remove(EntityID id) override;

    bool Has(EntityID id) const override;

    T& Get(EntityID id);

    const T& Get(EntityID id) const;
};
```

### Recommended Storage

For the first implementation:

```text
EntityID → Component
```

Possible implementation:

```cpp
std::unordered_map<EntityID, T>
```

Later optimization:

```text
Sparse Set
```

Structure:

```text
Sparse Set Component Pool

Entity IDs
    │
    ▼
Sparse Array
    │
    ▼
Dense Index
    │
    ▼
Dense Components
```

---

# 7. Component Manager

**Status: Planned**

File:

```text
ComponentManager.h
```

Responsibilities:

* Create component pools
* Store component pools
* Add components
* Remove components
* Get components
* Check component existence

### Expected API

```cpp
template<typename T>
void AddComponent(Entity entity, const T& component);

template<typename T>
void RemoveComponent(Entity entity);

template<typename T>
bool HasComponent(Entity entity) const;

template<typename T>
T& GetComponent(Entity entity);
```

### Internal Structure

```text
ComponentManager
        │
        ▼
ComponentTypeID
        │
        ▼
IComponentPool*
        │
        ├── ComponentPool<Position>
        ├── ComponentPool<Velocity>
        └── ComponentPool<Health>
```

---

# 8. ECS Registry / World

**Status: Planned**

File:

```text
Registry.h
```

or:

```text
World.h
```

This will be the main public ECS interface.

Recommended responsibilities:

```text
Registry
│
├── EntityManager
│
└── ComponentManager
```

### API

```cpp
Entity CreateEntity();

void DestroyEntity(Entity entity);

template<typename T>
void AddComponent(Entity entity, const T& component);

template<typename T>
void RemoveComponent(Entity entity);

template<typename T>
bool HasComponent(Entity entity) const;

template<typename T>
T& GetComponent(Entity entity);
```

### Example Usage

```cpp
ECS::Registry registry;

ECS::Entity player = registry.CreateEntity();

registry.AddComponent(
    player,
    Position{10.0f, 20.0f}
);

registry.AddComponent(
    player,
    Velocity{1.0f, 0.0f}
);
```

---

# 9. Entity Destruction Cleanup

**Important Step**

When an entity is destroyed:

```text
Destroy Entity
      │
      ▼
Remove Position
Remove Velocity
Remove Health
Remove Other Components
      │
      ▼
Increase Generation
      │
      ▼
Recycle Entity ID
```

The Registry should ensure all components belonging to the entity are removed.

Possible method:

```cpp
void RemoveEntityComponents(EntityID id);
```

This is where `IComponentPool` becomes useful.

---

# 10. Entity Queries

**Status: Future Step**

Allow finding entities with specific components.

Example:

```cpp
registry.View<Position, Velocity>();
```

Result:

```text
Entities with:

✓ Position
✓ Velocity
```

Example usage:

```cpp
for (Entity entity : registry.View<Position, Velocity>())
{
    auto& position = registry.GetComponent<Position>(entity);
    auto& velocity = registry.GetComponent<Velocity>(entity);

    position.x += velocity.x;
    position.y += velocity.y;
}
```

---

# 11. Systems

**Status: Future Step**

File:

```text
System.h
```

Base system:

```cpp
class System
{
public:
    virtual ~System() = default;

    virtual void Update(float deltaTime) = 0;
};
```

Example:

```text
System
│
├── MovementSystem
├── RenderSystem
├── PhysicsSystem
└── InputSystem
```

---

# 12. System Manager

**Status: Future Step**

Responsibilities:

* Register systems
* Initialize systems
* Update systems

Example:

```cpp
class SystemManager
{
public:

    template<typename T>
    void AddSystem();

    void Update(float deltaTime);
};
```

---

# 13. ECS Project Structure

Recommended structure:

```text
ECS/
│
├── Public/
│   │
│   ├── Types.h
│   ├── Entity.h
│   ├── EntityManager.h
│   ├── ComponentType.h
│   ├── IComponentPool.h
│   ├── ComponentPool.h
│   ├── ComponentManager.h
│   ├── Registry.h
│   │
│   ├── System.h
│   └── SystemManager.h
│
├── Private/
│   │
│   ├── EntityManager.cpp
│   ├── ComponentManager.cpp
│   ├── Registry.cpp
│   └── SystemManager.cpp
│
└── Tests/
    │
    ├── EntityTests.cpp
    ├── ComponentTests.cpp
    └── RegistryTests.cpp
```

---

# 14. Development Order

Recommended implementation order:

```text
[✓] Types
 │
 ▼
[✓] Entity
 │
 ▼
[✓] IComponentPool
 │
 ▼
[ ] EntityManager
 │
 ▼
[ ] ComponentTypeID Generator
 │
 ▼
[ ] ComponentPool<T>
 │
 ▼
[ ] ComponentManager
 │
 ▼
[ ] Registry
 │
 ▼
[ ] Entity Component Cleanup
 │
 ▼
[ ] Entity Views / Queries
 │
 ▼
[ ] Systems
 │
 ▼
[ ] SystemManager
 │
 ▼
[ ] Performance Optimization
```

---

# 15. Immediate Next Task

## Implement EntityManager

This should be the next implementation because everything depends on reliable entity creation and destruction.

Target API:

```cpp
namespace ECS
{
    class EntityManager
    {
    public:

        Entity Create();

        void Destroy(Entity entity);

        bool IsAlive(Entity entity) const;

        std::size_t GetAliveCount() const;

    private:

        // Generations for entity IDs
        // Recycled entity IDs
        // Alive entity count
    };
}
```

---

# Current ECS Progress

| Module            | Status      |
| ----------------- | ----------- |
| Types             | ✓ Completed |
| Entity            | ✓ Completed |
| IComponentPool    | ✓ Completed |
| EntityManager     | → Next      |
| Component Type ID | Planned     |
| ComponentPool     | Planned     |
| ComponentManager  | Planned     |
| Registry          | Planned     |
| Queries           | Planned     |
| Systems           | Planned     |
| Optimization      | Future      |

---

# Recommended Next Step

Implement:

```text
EntityManager.h
EntityManager.cpp
```

After that:

```text
ComponentType.h
```

Then:

```text
ComponentPool<T>
```

This creates a strong foundation before building the main `Registry`.
