/**
 * Registry.h
 *
 * The main public ECS interface. Combines EntityManager and ComponentManager
 * behind a single, simple API:
 *
 *   Registry registry;
 *
 *   Entity player = registry.CreateEntity();
 *   registry.AddComponent(player, Position{10.0f, 20.0f});
 *   registry.AddComponent(player, Velocity{1.0f, 0.0f});
 *
 *   if (Position* pos = registry.GetComponent<Position>(player))
 *   {
 *       pos->x += 1.0f;
 *   }
 *
 *   registry.DestroyEntity(player);
 *
 * Responsibilities:
 *   - Create and destroy entities (delegating to EntityManager).
 *   - Add/Remove/Has/Get components on entities (delegating to
 *     ComponentManager).
 *   - Ensure that destroying an entity also removes every component it
 *     owned, across every ComponentPool -- EntityManager alone only
 *     invalidates the Entity handle, it knows nothing about components.
 *
 * Internal structure:
 *
 *   Registry
 *       |
 *       +-- EntityManager     (entity lifetime, generations)
 *       +-- ComponentManager  (component storage, per type)
 */

#pragma once

#include "Types.h"
#include "Entity.h"
#include "EntityManager.h"
#include "ComponentManager.h"

#include <vector>
#include <algorithm>

namespace ECS
{
    class Registry
    {
    public:
        Registry() = default;

        /** Creates a new entity. Delegates to EntityManager, and tracks it as alive. */
        Entity CreateEntity()
        {
            Entity entity = m_entityManager.Create();
            m_aliveEntities.push_back(entity);
            return entity;
        }

        /**
         * Destroys an entity: removes every component it owned across all
         * pools, then invalidates its handle and recycles its EntityID.
         * A no-op if the entity is already stale or invalid.
         */
        void DestroyEntity(Entity entity)
        {
            if (!m_entityManager.IsAlive(entity))
            {
                return;
            }

            m_componentManager.RemoveEntityComponents(entity.id);
            m_entityManager.Destroy(entity);

            auto it = std::find(m_aliveEntities.begin(), m_aliveEntities.end(), entity);
            if (it != m_aliveEntities.end())
            {
                m_aliveEntities.erase(it);
            }
        }

        /** Returns true if the entity handle still refers to a currently-live entity. */
        bool IsAlive(Entity entity) const
        {
            return m_entityManager.IsAlive(entity);
        }

        /** Adds (or overwrites) a component of type T on the given entity. */
        template<typename T>
        void AddComponent(Entity entity, const T& component)
        {
            m_componentManager.AddComponent<T>(entity, component);
        }

        /** Removes the component of type T from the given entity, if present. */
        template<typename T>
        void RemoveComponent(Entity entity)
        {
            m_componentManager.RemoveComponent<T>(entity);
        }

        /** Returns true if the given entity has a component of type T. */
        template<typename T>
        bool HasComponent(Entity entity) const
        {
            return m_componentManager.HasComponent<T>(entity);
        }

        /** Returns a mutable pointer to the entity's component of type T, or nullptr if absent. */
        template<typename T>
        T* GetComponent(Entity entity)
        {
            return m_componentManager.GetComponent<T>(entity);
        }

        /** Returns a const pointer to the entity's component of type T, or nullptr if absent. */
        template<typename T>
        const T* GetComponent(Entity entity) const
        {
            return m_componentManager.GetComponent<T>(entity);
        }

        /**
         * Returns every currently-alive entity that has ALL of the given
         * component types.
         *
         * First implementation: walks every alive entity and checks
         * HasComponent<T>() for each requested type. O(aliveEntities *
         * componentTypes) per call -- simple and correct, not yet
         * optimized. A future version can back this with a sparse-set
         * intersection instead of a linear scan.
         *
         * Example:
         *   for (Entity entity : registry.View<Position, Velocity>())
         *   {
         *       auto* pos = registry.GetComponent<Position>(entity);
         *       auto* vel = registry.GetComponent<Velocity>(entity);
         *       pos->x += vel->x;
         *       pos->y += vel->y;
         *   }
         */
        template<typename... Components>
        std::vector<Entity> View() const
        {
            std::vector<Entity> result;

            for (const Entity& entity : m_aliveEntities)
            {
                if ((HasComponent<Components>(entity) && ...))
                {
                    result.push_back(entity);
                }
            }

            return result;
        }

    private:
        EntityManager      m_entityManager;
        ComponentManager   m_componentManager;
        std::vector<Entity> m_aliveEntities; ///< Every entity currently alive, in creation order.
    };
}