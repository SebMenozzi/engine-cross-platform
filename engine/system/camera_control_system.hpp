#pragma once

#include "coordinator.hpp"
#include "ecs_system.hpp"

#include "event.hpp"
#include "event_types.hpp"

#include "utils_maths.hpp"
#include "utils_types.hpp"

#include "transform.hpp"
#include "camera.hpp"
#include "rigid_body.hpp"
#include "gravity.hpp"

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
                void update_direction_();
                void orientate_with_mouse_();
                void move_from_keyboard_input_(float dt);
                void input_handler_(event::Event& event);
                void mouse_position_handler_(event::Event& event);
                void camera_angles_handler_(event::Event& event);

                Input input_;

                ecs::ECSEntity selected_;

                /// MacOS only: Internal mouse logic
                bool first_mouse_ = true;
                Diligent::float2 mouse_position_;
                Diligent::float2 last_mouse_position_ = Diligent::float2(0, 0);
                float mouse_sensitivity_ = 0.1;
                float move_velocity_ = 14.0;

                const Diligent::float3 up_axis_ = Diligent::float3(0, 1, 0);
        };
    }
}