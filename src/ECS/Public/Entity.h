/**
 * Entity handle: an Id plus a generation counter.
 * The generation lets the Registry detect "stale" handles -- if an entity
 * is destroyed and its Id is reused, any old Entity referencing the
 * previous generation is now invalid, even though the Id number matches.
 */

#pragma once

#include "Public/Types.h"

namespace ECS
{
    struct Entity
    {
        EntityID    id         = INVALID_ENTITY_ID;
        Generateion generation = 0;

        bool operator==(const Entity& other) const
        {
            return id == other.id && generation == other.generation;
        }

        bool IsValid() const { return id != INVALID_ENTITY_ID; }
    };
}
