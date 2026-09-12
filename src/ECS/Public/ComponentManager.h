/**
 * ComponentManager.h
 *
 * Owns one ComponentPool<T> per component type that has ever been used,
 * and routes Add/Remove/Has/Get calls to the correct pool based on T.
 *
 * Responsibilities:
 *   - Lazily create a ComponentPool<T> the first time type T is used.
 *   - Store all pools behind IComponentPool*, keyed by ComponentTypeID.
 *   - Add/Remove/Has/Get components for a given Entity and type T.
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
 */
#pragma once

#include "Public/Types.h"
#include "Public/Entity.h"

namespace ECS
{
    class ComponentManager
    {
    public:
        ComponentManager() = default;
        
        template<typename T>
        void AddComponent(Entity entity, const T& component)
        {
            GetOrCreatePool<T>().Add(entity.id, component);
        }

        template<typename T>
        void RemoveComponent(Entity entity) 
        {
            GetOrCreatePool<T>().Remove(entity.id);
        }

        template<typename T>
        bool HasComponent(Entity entity) const
        {
            return GetOrCreatePool<T>().Has(entity.id);
        }
        
    };

}
