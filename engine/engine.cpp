#include "engine.hpp"

namespace engine
{
    std::shared_ptr<Coordinator> coordinator = {};
    std::shared_ptr<graphics::GraphicsManager> graphics_manager = {};
    std::shared_ptr<system::PhysicsSystem> physics_system = {};
    std::shared_ptr<system::CameraControlSystem> camera_control_system = {};

    static bool quit = false;

    void quit_handler(event::Event& event)
    {
        quit = true;
    }

    void resize_handler(event::Event& event)
    {
        assert(graphics_manager);

        int width = event.get_parameter<int>(event::resize::WIDTH);
        int height = event.get_parameter<int>(event::resize::HEIGHT);

        graphics_manager->resize(width, height);
    }

    void Engine::init(
        Diligent::NativeWindow native_window,
        const std::string& assets_path
    )
    {
        coordinator = std::make_unique<Coordinator>();
        coordinator->init();
        coordinator->add_event_listener(EVENT_FUNCTION_LISTENER(event::QUIT, quit_handler));
        coordinator->add_event_listener(EVENT_FUNCTION_LISTENER(event::RESIZE, resize_handler));

        graphics_manager = std::make_unique<graphics::GraphicsManager>(assets_path);
        graphics_manager->initialize(&native_window);

        /// Register components

        coordinator->register_component<component::Transform>();
        coordinator->register_component<component::Camera>();
        coordinator->register_component<component::RigidBody>();
        coordinator->register_component<component::Gravity>();

        /// Systems

        physics_system = coordinator->register_system<system::PhysicsSystem>();
        {
            engine::ecs::ECSMask mask;
            mask.set(coordinator->get_component_type<component::Transform>());
            mask.set(coordinator->get_component_type<component::Gravity>());
            mask.set(coordinator->get_component_type<component::RigidBody>());
            coordinator->set_system_mask<system::PhysicsSystem>(mask);
        }

        camera_control_system = coordinator->register_system<system::CameraControlSystem>();
        {
            engine::ecs::ECSMask mask;
            mask.set(coordinator->get_component_type<component::Transform>());
            mask.set(coordinator->get_component_type<component::Camera>());
            mask.set(coordinator->get_component_type<engine::component::RigidBody>());
            coordinator->set_system_mask<system::CameraControlSystem>(mask);
        }

        camera_control_system->init();
    }

    void Engine::update(double dt)
    {
        assert(camera_control_system);
        assert(graphics_manager);
    
        camera_control_system->update(dt);
        physics_system->update(dt);

        Diligent::float4x4 view = camera_control_system->look_at();
        graphics_manager->set_camera_view(view);
        graphics_manager->update(dt);
    }

    void Engine::shutdown()
    {
        coordinator.reset();
        graphics_manager.reset();
        physics_system.reset();
        camera_control_system.reset();
    }

    bool Engine::should_quit()
    {
        return quit;
    }

    void Engine::send_event(event::Event& event)
    {
        assert(coordinator);

        coordinator->send_event(event);
    }
    
    void Engine::send_event(event::EventId event_id)
    {
        assert(coordinator);

        coordinator->send_event(event_id);
    }
}
