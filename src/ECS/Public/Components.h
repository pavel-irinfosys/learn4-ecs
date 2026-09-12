#pragma once

#include "Types.h"

namespace ECS
{
    struct PositionComponent
    {
        Float x = 0.0f;
        Float y = 0.0f;
    };

    struct HealthComponent
    {
        Float health = 100.0f;
    };

    struct VelocityComponent
    {
        Float x = 0.0f;
        Float y = 0.0f;
    };

}
