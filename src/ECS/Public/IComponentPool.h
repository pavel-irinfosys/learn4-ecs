/**
 * Type-erased interface for a component pool.
 *
 * Lets the Registry hold pools of different component types in a single
 * container (e.g. unordered_map<ComponentTypeId, unique_ptr<IComponentPool>>),
 * regardless of what T each ComponentPool<T> actually stores.
 */
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
