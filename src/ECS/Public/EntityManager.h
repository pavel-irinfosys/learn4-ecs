/**
 * EntityManager.h
 *
 * Manages the creation, destruction, and lifecycle of entities in the ECS.
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

        Entity Create();
        void Destroy(Entity entity);
        bool IsAlive(Entity entity) const;

    private:
        std::vector<Generation> m_generations;
        std::vector<EntityID> m_freeIDs;
    };
}
