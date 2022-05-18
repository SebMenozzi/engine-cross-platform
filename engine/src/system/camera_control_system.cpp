#include "camera_control_system.hpp"
#include "event_types.hpp"
#include "utils_types.hpp"

extern engine::Coordinator coordinator;

namespace engine
{
    namespace system
    {
        void CameraControlSystem::init()
        {
            coordinator.add_event_listener(EVENT_METHOD_LISTENER(event::INPUT, CameraControlSystem::input_handler_));
            coordinator.add_event_listener(EVENT_METHOD_LISTENER(event::MOUSE_POSITION, CameraControlSystem::mouse_position_handler_));

            // Create default camera

            selected_ = coordinator.create_entity();

            coordinator.add_component(
                selected_,
                component::Transform {
                    .position = Diligent::float3(0, 0, -5)
                }
            );

            coordinator.add_component(
                selected_,
                component::Camera {
                    .yaw = 0,
                    .pitch = 0
                }
            );

            setup_direction_();
        }

        void CameraControlSystem::update(float dt)
        {
            orientate_();

            auto& camera = coordinator.get_component<component::Camera>(selected_);
            auto& transform = coordinator.get_component<component::Transform>(selected_);

            double speed_up_scale = (input_.speed_up ? 2.0 : 1.0);

            if (input_.forward)
                transform.position += camera.direction * move_velocity_ * dt * speed_up_scale;

            if (input_.backward)
                transform.position -= camera.direction * move_velocity_ * dt * speed_up_scale;

            if (input_.left)
                transform.position += normalize(cross(camera.direction, up_axis_)) * dt * move_velocity_ * speed_up_scale;

            if (input_.right)
                transform.position -= normalize(cross(camera.direction, up_axis_)) * dt * move_velocity_ * speed_up_scale;

            if (input_.up)
                transform.position.y += move_velocity_ * dt * speed_up_scale;
        }

        Diligent::float4x4 CameraControlSystem::look_at()
        {
            auto& camera = coordinator.get_component<component::Camera>(selected_);
            auto& transform = coordinator.get_component<component::Transform>(selected_);

            Diligent::float3 z_axis = camera.direction;
            Diligent::float3 x_axis = normalize(cross(up_axis_, z_axis));
            Diligent::float3 y_axis = cross(z_axis, x_axis);

            return Diligent::float4x4(
                x_axis.x,                y_axis.x,                z_axis.x,                0,
                x_axis.y,                y_axis.y,                z_axis.y,                0,
                x_axis.z,                y_axis.z,                z_axis.z,                0,
                -dot(x_axis, transform.position), -dot(y_axis, transform.position), -dot(z_axis, transform.position), 1
            );
        }

        // MARK: - Private methods

        void CameraControlSystem::orientate_()
        {
            auto& camera = coordinator.get_component<component::Camera>(selected_);

            if (first_mouse_)
            {
                last_mouse_position_ = mouse_position_;
                first_mouse_ = false;
            }

            double x_offset = mouse_position_.x - last_mouse_position_.x;
            double y_offset = last_mouse_position_.y - mouse_position_.y;

            last_mouse_position_ = mouse_position_;

            x_offset *= mouse_sensitivity_;
            y_offset *= mouse_sensitivity_;

            camera.yaw += x_offset;
            camera.pitch += y_offset; 

            setup_direction_();
        }

        void CameraControlSystem::setup_direction_()
        {
            auto& camera = coordinator.get_component<component::Camera>(selected_);

            camera.pitch = utils::clamp(camera.pitch, -89, 89);

            float yaw_radian = utils::degrees_to_radians(camera.yaw);
            float pitch_radian = utils::degrees_to_radians(camera.pitch);

            if (up_axis_.x == 1.0)
            {
                camera.direction.x = sin(pitch_radian);
                camera.direction.y = cos(pitch_radian) * cos(yaw_radian);
                camera.direction.z = cos(pitch_radian) * sin(yaw_radian);
            }
            else if(up_axis_.y == 1.0)
            {
                camera.direction.x = cos(pitch_radian) * sin(yaw_radian);
                camera.direction.y = sin(pitch_radian);
                camera.direction.z = cos(pitch_radian) * cos(yaw_radian);
            } 
            else
            {
                camera.direction.x = cos(pitch_radian) * cos(yaw_radian);
                camera.direction.y = cos(pitch_radian) * sin(yaw_radian);
                camera.direction.z = sin(pitch_radian);
            }

            camera.direction = normalize(camera.direction);
        }

        void CameraControlSystem::input_handler_(event::Event& event)
        {
            input_ = event.get_parameter<Input>(event::input::PARAMETER);
        }

        void CameraControlSystem::mouse_position_handler_(event::Event& event)
        {
            double x = event.get_parameter<double>(engine::event::mouse_position::X);
	        double y = event.get_parameter<double>(engine::event::mouse_position::Y);

            mouse_position_ = Diligent::float2(x, y);
        }
    }
}