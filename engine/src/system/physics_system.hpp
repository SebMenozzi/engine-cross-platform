#pragma once

#include "coordinator.hpp"

#include "transform.hpp"
#include "rigid_body.hpp"
#include "gravity.hpp"

#include "utils_types.hpp"

#include "event.hpp"
#include "event_types.hpp"

namespace engine
{
    namespace system
    {
        class PhysicsSystem : public ecs::ECSSystem
        {
            public:
                void update(float dt);
            private:
                void input_handler_(event::Event& event);

                bool gravity_enabled_ = false;
        };
    }
}