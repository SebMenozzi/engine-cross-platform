#pragma once

#include "object.hpp"

namespace engine
{
    namespace object
    {
        // MARK: - Cube Constants

        //     (-1,+1,+1) ________________ (+1,+1,+1)                 Z
        //               /|              /|                           |      Y
        //              / |             / |                           |     /
        //             /  |            /  |                           |    /
        //            /   |           /   |                           |   /
        //(-1,-1,+1) /____|__________/ (+1,-1,+1)                     |  /
        //           |    |__________|____|                           | /
        //           |   / (-1,+1,-1)|    / (+1,+1,-1)                |----------------> X
        //           |  /            |   /
        //           | /             |  /
        //           |/              | /
        //           /_______________|/
        //        (-1,-1,-1)       (+1,-1,-1)
        //

        // MARK: - Public Methods

        const std::vector<Diligent::float3> CUBE_POSITIONS = {
            Diligent::float3{-1, -1, -1}, Diligent::float3{-1, +1, -1}, Diligent::float3{+1, +1, -1}, Diligent::float3{+1, -1, -1}, // Bottom
            Diligent::float3{-1, -1, -1}, Diligent::float3{-1, -1, +1}, Diligent::float3{+1, -1, +1}, Diligent::float3{+1, -1, -1}, // Front
            Diligent::float3{+1, -1, -1}, Diligent::float3{+1, -1, +1}, Diligent::float3{+1, +1, +1}, Diligent::float3{+1, +1, -1}, // Right
            Diligent::float3{+1, +1, -1}, Diligent::float3{+1, +1, +1}, Diligent::float3{-1, +1, +1}, Diligent::float3{-1, +1, -1}, // Back
            Diligent::float3{-1, +1, -1}, Diligent::float3{-1, +1, +1}, Diligent::float3{-1, -1, +1}, Diligent::float3{-1, -1, -1}, // Left
            Diligent::float3{-1, -1, +1}, Diligent::float3{+1, -1, +1}, Diligent::float3{+1, +1, +1}, Diligent::float3{-1, +1, +1}  // Top
        };

        const std::vector<Diligent::float3> CUBE_NORMALS = {
            Diligent::float3{0, 0, -1}, Diligent::float3{0, 0, -1}, Diligent::float3{0, 0, -1}, Diligent::float3{0, 0, -1}, // Bottom
            Diligent::float3{0, -1, 0}, Diligent::float3{0, -1, 0}, Diligent::float3{0, -1, 0}, Diligent::float3{0, -1, 0}, // Front
            Diligent::float3{+1, 0, 0}, Diligent::float3{+1, 0, 0}, Diligent::float3{+1, 0, 0}, Diligent::float3{+1, 0, 0}, // Right
            Diligent::float3{0, +1, 0}, Diligent::float3{0, +1, 0}, Diligent::float3{0, +1, 0}, Diligent::float3{0, +1, 0}, // Back
            Diligent::float3{-1, 0, 0}, Diligent::float3{-1, 0, 0}, Diligent::float3{-1, 0, 0}, Diligent::float3{-1, 0, 0}, // Left
            Diligent::float3{0, 0, +1}, Diligent::float3{0, 0, +1}, Diligent::float3{0, 0, +1}, Diligent::float3{0, 0, +1}  // Top
        };

        const std::vector<Diligent::float2> CUBE_TEXTCOORDS = {
            Diligent::float2{0, 1}, Diligent::float2{0, 0}, Diligent::float2{1, 0}, Diligent::float2{1, 1}, // Bottom
            Diligent::float2{0, 1}, Diligent::float2{0, 0}, Diligent::float2{1, 0}, Diligent::float2{1, 1}, // Front
            Diligent::float2{0, 1}, Diligent::float2{1, 1}, Diligent::float2{1, 0}, Diligent::float2{0, 0}, // Right
            Diligent::float2{0, 1}, Diligent::float2{0, 0}, Diligent::float2{1, 0}, Diligent::float2{1, 1}, // Back
            Diligent::float2{1, 0}, Diligent::float2{0, 0}, Diligent::float2{0, 1}, Diligent::float2{1, 1}, // Left
            Diligent::float2{1, 1}, Diligent::float2{0, 1}, Diligent::float2{0, 0}, Diligent::float2{1, 0}  // Top
        };

        const std::vector<Diligent::Uint32> CUBE_INDICES =
        {
            2,0,1,    2,3,0,
            4,6,5,    4,7,6,
            8,10,9,   8,11,10,
            12,14,13, 12,15,14,
            16,18,17, 16,19,18,
            20,21,22, 20,22,23
        };
    }
}