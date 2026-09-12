/**
 * ComponentType.h
 *
 * Generates a unique, stable ComponentTypeID for every distinct component
 * type used with the ECS.
 *
 * Usage:
 *     struct Position {};
 *     struct Velocity {};
 *
 *     ComponentTypeID posId = GetComponentTypeID<Position>();
 *     ComponentTypeID velId = GetComponentTypeID<Velocity>();
 *
 * Position and Velocity will each receive a distinct id (0, 1, 2, ...),
 * assigned in the order they are first requested. The same type always
 * returns the same id for the lifetime of the program.
 *
 * How it works:
 *   GetComponentTypeID<T>() is a function template. Each distinct T causes
 *   the compiler to instantiate a separate copy of the function, and each
 *   copy owns its own `static const ComponentTypeID id`. A local static is
 *   initialized exactly once, the first time control passes through it, so
 *   the very first call for a given T assigns it the next available id
 *   (via GetNextComponentTypeID()), and every later call for that same T
 *   just returns the already-initialized value.
 *
 *   GetNextComponentTypeID() itself uses the same trick with a single
 *   shared static counter to hand out sequential ids across all types.
 */

#pragma once

#include "Types.h"

namespace ECS
{
    namespace Internal
    {
        /** Shared counter that hands out the next unused ComponentTypeID. */
        inline ComponentTypeID GetNextComponentTypeID()
        {
            static ComponentTypeID nextID = 0;
            return nextID++;
        }
    }

    /**
     * Returns the unique ComponentTypeID associated with type T.
     * The id is assigned the first time this is called for a given T,
     * and is stable for the remainder of the program's execution.
     */
    template<typename T>
    ComponentTypeID GetComponentTypeID()
    {
        static const ComponentTypeID id = Internal::GetNextComponentTypeID();
        return id;
    }
}