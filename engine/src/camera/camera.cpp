#include "camera.hpp"

namespace engine
{
    namespace camera
    {
        // MARK: - Public

        Camera::Camera(
            Diligent::float3 position,
            Diligent::float3 up_axis,
            float mouse_sensitivity,
            float move_velocity
        ): 
            yaw_(0),
            pitch_(0),
            position_(position),
            up_axis_(up_axis),
            mouse_sensitivity_(mouse_sensitivity),
            move_velocity_(move_velocity)
        {
            setup_direction_();
        }

        Camera::~Camera() {}

        void Camera::set_mouse_position(Diligent::float2 mouse_position)
        {
            mouse_position_ = mouse_position;
        }

        void Camera::set_move_direction(Direction move_direction)
        {
            move_direction_ = move_direction;
        }

        void Camera::set_is_speed_up(bool is_speed_up)
        {
            is_speed_up_ = is_speed_up;
        }

        void Camera::toggle_gravity()
        {
            gravity_enabled_ = !gravity_enabled_;
        }

        void Camera::update(double dt)
        {
            orientate_(mouse_position_.x, mouse_position_.y);

            // Forward
            if (move_direction_.forward)
                position_ += direction_ * move_velocity_ * (is_speed_up_ ? 2.0 : 1.0);
            // Backward
            if (move_direction_.backward)
                position_ -= direction_ * move_velocity_ * (is_speed_up_ ? 2.0 : 1.0);
            // Left
            if (move_direction_.left)
                position_ += normalize(cross(direction_, up_axis_)) * move_velocity_ * (is_speed_up_ ? 2.0 : 1.0);
            // Right
            if (move_direction_.right)
                position_ -= normalize(cross(direction_, up_axis_)) * move_velocity_ * (is_speed_up_ ? 2.0 : 1.0);
            // up
            if (move_direction_.up)
                position_.y += move_velocity_ * (is_speed_up_ ? 2.0 : 1.0);
            
            // TODO: move gravity logic outside the camera
            if (gravity_enabled_)
            {
                position_.x += velocity_.x * dt;
                position_.y += velocity_.y * dt;

                if (position_.y <= 0)
                {
                    position_.y = 0;
                    velocity_.y = 0;
                }

                double friction = 0.02;

                velocity_.x *= exp(-friction * dt);
                velocity_.y -= gravity_ * dt;
            }
            else
                velocity_ = Diligent::float2(0, 0);
        }

        Diligent::float4x4 Camera::look_at()
        {
            Diligent::float3 z_axis = direction_;
            Diligent::float3 x_axis = normalize(cross(up_axis_, z_axis));
            Diligent::float3 y_axis = cross(z_axis, x_axis);

            return Diligent::float4x4(
                x_axis.x,                y_axis.x,                z_axis.x,                0,
                x_axis.y,                y_axis.y,                z_axis.y,                0,
                x_axis.z,                y_axis.z,                z_axis.z,                0,
                -dot(x_axis, position_), -dot(y_axis, position_), -dot(z_axis, position_), 1
            );
        }

        // MARK: - Private

        void Camera::orientate_(double x, double y)
        {
            if (first_mouse_)
            {
                last_mouse_position_ = Diligent::float2(x, y);
                first_mouse_ = false;
            }

            double x_offset = x - last_mouse_position_.x;
            double y_offset = last_mouse_position_.y - y;

            last_mouse_position_ = Diligent::float2(x, y);

            x_offset *= mouse_sensitivity_;
            y_offset *= mouse_sensitivity_;

            yaw_ += x_offset;
            pitch_ += y_offset; 

            setup_direction_();
        }

        void Camera::setup_direction_()
        {
            pitch_ = utils::clamp(pitch_, -89, 89);

            float yaw_radian = utils::degrees_to_radians(yaw_);
            float pitch_radian = utils::degrees_to_radians(pitch_);

            if (up_axis_.x == 1.0)
            {
                direction_.x = sin(pitch_radian);
                direction_.y = cos(pitch_radian) * cos(yaw_radian);
                direction_.z = cos(pitch_radian) * sin(yaw_radian);
            }
            else if(up_axis_.y == 1.0)
            {
                direction_.x = cos(pitch_radian) * sin(yaw_radian);
                direction_.y = sin(pitch_radian);
                direction_.z = cos(pitch_radian) * cos(yaw_radian);
            } 
            else
            {
                direction_.x = cos(pitch_radian) * cos(yaw_radian);
                direction_.y = cos(pitch_radian) * sin(yaw_radian);
                direction_.z = sin(pitch_radian);
            }

            direction_ = normalize(direction_);
        }
    }
}