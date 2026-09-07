/**
 * 
 */

#pragma once

#include <cstdint>
#include <cstddef>

namespace ECS
{
    using EntityID          = std::uint32_t;
    using Generation        = std::uint32_t;
    using ComponentTypeID   = std::size_t;
    using EntityCount       = std::size_t;

    constexpr EntityID INVALID_ENTITY_ID = 0;
}
