#pragma once

#include <BasicMath.hpp>

namespace engine
{
    namespace component
    {
        struct Camera
        {
            double yaw;
            double pitch;
            double roll;

            Diligent::float3 direction;
        };
    }
}