#pragma once

#include "coordinator.hpp"

#include "transform.hpp"
#include "rigid_body.hpp"
#include "gravity.hpp"

namespace engine
{
    namespace system
    {
        class PhysicsSystem : public ecs::ECSSystem
        {
            public:
                void update(float dt);
        };
    }
}