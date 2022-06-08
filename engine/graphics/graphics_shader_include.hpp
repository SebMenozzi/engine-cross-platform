#pragma once

namespace Diligent
{
    // We only need a 3x3 matrix, but in Vulkan and Metal, the rows of a float3x3 matrix are aligned to 16 bytes,
    // which is effectively a float4x3 matrix.
    // In DirectX, the rows of a float3x3 matrix are not aligned.
    // We will use a float4x3 for compatibility between all APIs.
    struct float4x3
    {
        float m00 = 0.f;
        float m01 = 0.f;
        float m02 = 0.f;
        float m03 = 0.f; // Unused

        float m10 = 0.f;
        float m11 = 0.f;
        float m12 = 0.f;
        float m13 = 0.f; // Unused

        float m20 = 0.f;
        float m21 = 0.f;
        float m22 = 0.f;
        float m23 = 0.f; // Unused

        float4x3() {}

        template <typename MatType>
        float4x3(const MatType& other) :
            m00{other.m00}, m01{other.m01}, m02{other.m02}, 
            m10{other.m10}, m11{other.m11}, m12{other.m12}, 
            m20{other.m20}, m21{other.m21}, m22{other.m22}
        {}
    };

    #include "assets/shaders/structures.fxh"
} 
