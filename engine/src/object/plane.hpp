#pragma once

#include "object.hpp"

namespace engine
{
    namespace object
    {
        // MARK: - Plane Constants
        
        //
        //      (-1,0,-1) _________________ (+1,0,-1)
        //               /                /            
        //              /                /
        //             /                /
        //            /                /
        //           /________________/
        //       (-1,0,-1)        (+1,0,-1)
        //

        // MARK: - Public Methods

        const std::vector<Diligent::float3> PLANE_POSITIONS = {
            Diligent::float3{-10, -1, -10}, 
            Diligent::float3{+10, -1, -10}, 
            Diligent::float3{-10, -1, +10}, 
            Diligent::float3{+10, -1, +10}
        };

        const std::vector<Diligent::float3> PLANE_NORMALS = {
            Diligent::float3{0, 1, 0},
            Diligent::float3{0, 1, 0},
            Diligent::float3{0, 1, 0},
            Diligent::float3{0, 1, 0}
        };

        const std::vector<Diligent::float2> PLANE_TEXTCOORDS = {
            Diligent::float2{0, 0},
            Diligent::float2{1, 0},
            Diligent::float2{0, 1},
            Diligent::float2{1, 1},
        };

        const std::vector<Diligent::Uint32> PLANE_INDICES = {0, 2, 3, 3, 1, 0};
    }
}