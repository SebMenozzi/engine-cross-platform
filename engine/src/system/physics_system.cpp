#include "physics_system.hpp"

namespace engine
{
    extern std::shared_ptr<Coordinator> coordinator;

    namespace system
    {
        void PhysicsSystem::update(float dt)
        {
            assert(coordinator);

            coordinator->add_event_listener(EVENT_METHOD_LISTENER(event::INPUT, PhysicsSystem::input_handler_));

            if (gravity_enabled_)
            {
                for (auto const& entity : entities_)
                {
                    auto& rigid_body = coordinator->get_component<component::RigidBody>(entity);
                    auto& transform = coordinator->get_component<component::Transform>(entity);
                    auto const& gravity = coordinator->get_component<component::Gravity>(entity);

                    // Force
                    transform.position += rigid_body.velocity * dt;
                    rigid_body.velocity += gravity.force * dt;
                }
            }
        }

        void PhysicsSystem::input_handler_(event::Event& event)
        {
            Input input = event.get_parameter<Input>(event::input::PARAMETER);
            gravity_enabled_ = input.gravity;
        }
    }
}
