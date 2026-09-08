/**
 * 
 */

#include "Public/Types.h"
#include "Public/Entity.h"

#include "EntityManager.h"

namespace ECS
{
    Entity EntityManager::Create()
    {
        if (m_freeIDs.empty())
        {
            EntityID id = m_freeIDs.back();
            m_freeIDs.pop_back();

            return Entity { id, m_generations[id] };
        }

        EntityID id = static_cast<EntityID>(m_generations.size());
        m_generations.push_back(0);

        return Entity { id, 0 };
    }

    void EntityManager::Destroy(Entity entity) {
        
        if (!IsAlive(entity))
        {
            return;
        }

        ++m_generations[entity.id];
        m_freeIDs.push_back(entity.id);
    }
    

    bool EntityManager::IsAlive(Entity entity) const
    {
        if (!entity.IsValid())
        {
            return false;
        }

        if (entity.id >= m_generations.size())
        {
            return false;
        }

        return m_generations[entity.id] == entity.generation;
    }
}

