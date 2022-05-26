
#pragma once

#include <vector>
#include <BasicMath.hpp>

namespace engine
{
    namespace object
    {
        namespace sphere
        {
            const int MIN_SECTOR_COUNT = 3;
            const int MIN_STACK_COUNT = 2;

            class UVSphere
            {
                public:
                    UVSphere(
                        float radius, 
                        int sectorCount, 
                        int stackCount
                    );

                    std::vector<Diligent::float3> vertices_;
                    std::vector<Diligent::float3> normals_;
                    std::vector<Diligent::float2> textcoords_;
                    std::vector<Diligent::Uint32> indices_;

                protected:
                    float radius_;
                    int sectorCount_;
                    int stackCount_;
            };
        }
    }
}