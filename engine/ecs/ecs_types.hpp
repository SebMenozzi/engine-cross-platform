#pragma once

#include <bitset>

namespace engine
{
    namespace ecs
    {
        using ECSEntity = std::uint32_t;
        const ECSEntity MAX_ENTITIES = 5000;

        using ECSComponentType = std::uint8_t;
        const ECSComponentType MAX_COMPONENTS = 32;

        using ECSMask = std::bitset<MAX_COMPONENTS>;
    }
}