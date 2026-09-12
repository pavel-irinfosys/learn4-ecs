/**
 * EntityManager.h
 *
 * Manages the creation, destruction, and lifecycle of entities in the ECS.
 */
#pragma once

#include "Types.h"
#include "Entity.h"

#include <vector>

namespace ECS
{
    class EntityManager
    {
    public:
        EntityManager()
        {
            // Slot 0 is reserved for INVALID_ENTITY_ID (0).
            // No real entity ever receives id 0, so the first entity created
            // gets id 1, and m_generations[0] stays a permanent dummy slot.
            m_generations.push_back(0);
        }   

        Entity Create();
        void Destroy(Entity entity);
        bool IsAlive(Entity entity) const;

    private:
        std::vector<Generation> m_generations;
        std::vector<EntityID> m_freeIDs;
    };
}
