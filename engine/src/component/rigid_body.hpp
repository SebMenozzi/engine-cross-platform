#pragma once

#include <BasicMath.hpp>

namespace engine
{
    namespace component
    {
        struct RigidBody
        {
            Diligent::float3 velocity;
            Diligent::float3 acceleration;
        };
    }
}