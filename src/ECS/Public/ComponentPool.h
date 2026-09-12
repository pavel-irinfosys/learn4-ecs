/**
 * ComponentPool.h
 *
 * Stores components of a single type T, keyed by EntityID.
 *
 * ComponentPool<T> implements IComponentPool so that a ComponentManager can
 * hold pools of different component types behind a single common pointer
 * type (IComponentPool*), while still allowing type-safe access through
 * ComponentPool<T> directly when T is known.
 *
 * Storage:
 *   std::unordered_map<EntityID, T> -- simple and correct for a first
 *   implementation. A future optimization is to replace this with a sparse
 *   set for better cache locality and iteration performance.
 */

#pragma once

#include "Public/Types.h"
#include "Public/IComponentPool.h"

#include <unordered_map>
#include <cassert>

namespace ECS
{
    template<typename T>
    class ComponentPool : public IComponentPool
    {
    public:
        ComponentPool() = default;
 
        /** Adds or overwrites the component belonging to the given EntityID. */
        void Add(EntityID id, const T& component)
        {
            m_components[id] = component;
        }
 
        /** Removes the component belonging to the given EntityID, if any. Safe to call if absent. */
        void Remove(EntityID id) override
        {
            m_components.erase(id);
        }
 
        /** Returns true if this pool holds a component for the given EntityID. */
        bool Has(EntityID id) const override
        {
            return m_components.find(id) != m_components.end();
        }
 
        /** Returns a mutable pointer to the component for the given EntityID, or nullptr if absent. */
        T* Get(EntityID id)
        {
            auto it = m_components.find(id);
            if (it == m_components.end())
            {
                return nullptr;
            }
            return &it->second;
        }
 
        /** Returns a const pointer to the component for the given EntityID, or nullptr if absent. */
        const T* Get(EntityID id) const
        {
            auto it = m_components.find(id);
            if (it == m_components.end())
            {
                return nullptr;
            }
            return &it->second;
        }
 
    private:
        std::unordered_map<EntityID, T> m_components;
    };
}