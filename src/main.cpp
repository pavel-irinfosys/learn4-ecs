/**
 * main file
 */
#include <SDL3/SDL.h>
#include <cstdio>

#include "Types.h"
#include "Registry.h"
#include "Components.h"



int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    
    ECS::Registry registry;   
    ECS::Entity a = registry.CreateEntity();
    registry.AddComponent(a, ECS::PositionComponent{ 10.0f, 20.0f });
    registry.AddComponent(a, ECS::HealthComponent{ 100.0f });
    
    printf("Created entity with ID: %u, Generation: %u\n", a.id, a.generation);
    if (auto* pos = registry.GetComponent<ECS::PositionComponent>(a))
    {
        printf("Position: (%f, %f)\n", pos->x, pos->y);
    }

    if (auto* health = registry.GetComponent<ECS::HealthComponent>(a))
    {
        printf("Health: %f\n", health->health);
    }

    if (auto* vel = registry.GetComponent<ECS::VelocityComponent>(a))
    {
        printf("Velocity: (%f, %f)\n", vel->x, vel->y);
    }

    return 0;
}
