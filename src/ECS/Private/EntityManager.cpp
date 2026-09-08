/**
 * EntityManager: owns the lifetime of every Entity in the ECS.
 *
 * Responsibilities:
 *   - Create()   allocates a new Entity, reusing a recycled Id when possible.
 *   - Destroy()  invalidates an Entity by bumping its generation, then
 *                recycles the Id for future reuse.
 *   - IsAlive()  checks whether a given Entity handle still matches the
 *                current generation stored for its Id.
 *
 * Ids are never freed from memory; they are recycled. Each Id has an
 * associated generation counter that increases every time the Id is
 * destroyed and reused, which is what allows stale Entity handles to be
 * detected after their Id has been reassigned to a new entity.
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

