#include "physics_system.hpp"

namespace engine
{
    extern std::shared_ptr<Coordinator> coordinator;

    namespace system
    {
        void PhysicsSystem::update(float dt)
        {
            assert(coordinator);

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
}