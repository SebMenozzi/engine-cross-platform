#pragma once

#include <BasicMath.hpp>

namespace engine
{
    namespace component
    {
        struct Camera
        {
            /// Horizontal angle
            double yaw;
            /// Vertical angle
            double pitch;

            Diligent::float3 direction;
        };
    }
}