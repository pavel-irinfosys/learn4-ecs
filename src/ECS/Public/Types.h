/**
 * 
 */

#pragma once

#include <cstdint>
#include <cstddef>

namespace ECS
{
    using EntityID          = std::uint32_t;
    using Generateion       = std::uint32_t;
    using ComponentTypeID   = std::size_t;

    constexpr EntityID INVALID_ENTITY_ID = 0;
}
