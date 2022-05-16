
   
#pragma once

#include <BasicMath.hpp>

#include "maths_utils.hpp"

namespace engine
{
    namespace camera
    {
        struct Direction
        {
            bool forward = false;
            bool backward = false;
            bool left = false;
            bool right = false;
            bool up = false;
        };

        class Camera
        {
            public:
                Camera(
                    Diligent::float3 position,
                    Diligent::float3 up_axis,
                    float sensitivity,
                    float velocity
                );

                ~Camera();

                void set_mouse_position(Diligent::float2 mouse_position);
                void set_move_direction(Direction move_direction);
                void set_is_speed_up(bool is_speed_up);
                void toggle_gravity();
                void update(double dt);
                Diligent::float4x4 look_at();

            private:
                /// Camera inputs
                Direction move_direction_;
                Diligent::float2 mouse_position_;
                bool is_speed_up_;

                /// Camera angles
                double yaw_; // horizontal angle
                double pitch_; // vertical angle

                /// Camera parameters
                Diligent::float3 position_;
                Diligent::float3 up_axis_;
                float mouse_sensitivity_;
                float move_velocity_;

                Diligent::float3 direction_;

                /// Internal mouse logic
                bool first_mouse_ = true;
                Diligent::float2 last_mouse_position_ = Diligent::float2(0, 0);

                /// Gravity
                double gravity_ = 9.81;
                Diligent::float2 velocity_ = Diligent::float2(0, 0);
                bool gravity_enabled_ = false;

                /// Private methods
                void setup_direction_();
                void orientate_(double x, double y);
        };
    }
}