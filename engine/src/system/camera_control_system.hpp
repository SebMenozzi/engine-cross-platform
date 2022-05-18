#pragma once

#include "transform.hpp"
#include "camera.hpp"
#include "coordinator.hpp"
#include "ecs_system.hpp"
#include "event_types.hpp"
#include "event.hpp"
#include "utils_types.hpp"
#include "utils_maths.hpp"

namespace engine
{
    namespace system
    {
        class CameraControlSystem : public ecs::ECSSystem
        {
            public:
                void init();
                void update(float dt);
                Diligent::float4x4 look_at();
            private:
                void setup_direction_();
                void orientate_();
                void input_handler_(event::Event& event);
                void mouse_position_handler_(event::Event& event);

                Input input_;

                ecs::ECSEntity selected_;

                /// Internal mouse logic
                bool first_mouse_ = true;
                Diligent::float2 mouse_position_;
                Diligent::float2 last_mouse_position_ = Diligent::float2(0, 0);

                /// Constants

                Diligent::float3 up_axis_ = Diligent::float3(0, 1, 0);
                float mouse_sensitivity_ = 0.1;
                float move_velocity_ = 0.02;
        };
    }
}