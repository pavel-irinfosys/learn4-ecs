/**
 * ComponentManager.h
 *
 * Owns one ComponentPool<T> per component type that has ever been used,
 * and routes Add/Remove/Has/Get calls to the correct pool based on T.
 *
 * Responsibilities:
 *   - Lazily create a ComponentPool<T> the first time type T is added.
 *   - Store all pools behind IComponentPool*, keyed by ComponentTypeID.
 *   - Add/Remove/Has/Get components for a given Entity and type T.
 *     GetComponent() returns a pointer (T* / const T*), which is nullptr
 *     if the entity has no component of that type -- it never creates a
 *     pool or asserts.
 *   - Remove every component belonging to an entity across all pools,
 *     for use when an entity is destroyed.
 *
 * Internal structure:
 *
 *   ComponentManager
 *           |
 *           v
 *   ComponentTypeID -> IComponentPool*
 *           |
 *           +-- ComponentPool<Position>
 *           +-- ComponentPool<Velocity>
 *           +-- ComponentPool<Health>
 *
 * Note: AddComponent/RemoveComponent/HasComponent/GetComponent are function
 * templates, so their definitions must stay visible in this header (the
 * compiler needs the full body at every call site to instantiate them for
 * each T). RemoveEntityComponents() is not a template, so its definition
 * lives in ComponentManager.cpp instead.
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
 
        /** Adds (or overwrites) a component of type T on the given entity. */
        template<typename T>
        void AddComponent(Entity entity, const T& component)
        {
            GetOrCreatePool<T>().Add(entity.id, component);
        }
 
        /** Removes the component of type T from the given entity, if present. */
        template<typename T>
        void RemoveComponent(Entity entity)
        {
            if (IComponentPool* pool = FindPool<T>())
            {
                pool->Remove(entity.id);
            }
        }
 
        /** Returns true if the given entity has a component of type T. */
        template<typename T>
        bool HasComponent(Entity entity) const
        {
            const IComponentPool* pool = FindPool<T>();
            return pool != nullptr && pool->Has(entity.id);
        }
 
        /**
         * Returns a mutable pointer to the entity's component of type T,
         * or nullptr if the entity has no component of that type (or no
         * pool for T has ever been created). Does not create a pool.
         */
        template<typename T>
        T* GetComponent(Entity entity)
        {
            IComponentPool* pool = FindPool<T>();
            if (pool == nullptr)
            {
                return nullptr;
            }
 
            return static_cast<ComponentPool<T>*>(pool)->Get(entity.id);
        }
 
        /**
         * Returns a const pointer to the entity's component of type T,
         * or nullptr if the entity has no component of that type (or no
         * pool for T has ever been created).
         */
        template<typename T>
        const T* GetComponent(Entity entity) const
        {
            const IComponentPool* pool = FindPool<T>();
            if (pool == nullptr)
            {
                return nullptr;
            }
 
            return static_cast<const ComponentPool<T>*>(pool)->Get(entity.id);
        }
 
        /**
         * Removes every component belonging to the given EntityID, across
         * every pool. Intended to be called when an entity is destroyed,
         * before its EntityID is recycled. Defined inline here (not a
         * template, but still safe to include from multiple .cpp files).
         */
        void RemoveEntityComponents(EntityID id)
        {
            for (const auto& [typeID, pool] : m_pools)
            {
                pool->Remove(id);
            }
        }
 
    private:
        /** Returns the pool for T, creating it the first time T is used. */
        template<typename T>
        ComponentPool<T>& GetOrCreatePool()
        {
            const ComponentTypeID typeID = GetComponentTypeID<T>();
 
            auto it = m_pools.find(typeID);
            if (it == m_pools.end())
            {
                it = m_pools.emplace(typeID, std::make_unique<ComponentPool<T>>()).first;
            }
 
            return static_cast<ComponentPool<T>&>(*it->second);
        }
 
        /** Returns the pool for T, or nullptr if T has never been used. */
        template<typename T>
        IComponentPool* FindPool()
        {
            const ComponentTypeID typeID = GetComponentTypeID<T>();
            auto it = m_pools.find(typeID);
            return it != m_pools.end() ? it->second.get() : nullptr;
        }
 
        template<typename T>
        const IComponentPool* FindPool() const
        {
            const ComponentTypeID typeID = GetComponentTypeID<T>();
            auto it = m_pools.find(typeID);
            return it != m_pools.end() ? it->second.get() : nullptr;
        }
 
        std::unordered_map<ComponentTypeID, std::unique_ptr<IComponentPool>> m_pools;
    };
}