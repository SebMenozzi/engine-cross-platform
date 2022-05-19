#include <iostream>

#include "coordinator.hpp"
#include "graphics_manager.hpp"
#include "window_manager.hpp"

#include "gravity.hpp"
#include "camera.hpp"
#include "rigid_body.hpp"
#include "transform.hpp"

#include "utils_maths.hpp"
#include "coordinator.hpp"
#include "event_types.hpp"

//#include "camera_control_system.hpp"
//#include "physics_system.hpp"

std::shared_ptr<engine::coordinator::Coordinator> coordinator = {};
std::shared_ptr<engine::graphics::GraphicsManager> graphics_manager = {};
std::shared_ptr<desktop::window::WindowManager> window_manager = {};

static bool quit = false;

void quit_handler(engine::event::Event& event)
{
	quit = true;
}

void resize_handler(engine::event::Event& event)
{
    int width = event.get_parameter<int>(engine::event::resize::WIDTH);
	int height = event.get_parameter<int>(engine::event::resize::HEIGHT);

	graphics_manager->resize(width, height);
}

int main(int argc, char *argv[])
{
    coordinator = std::make_unique<engine::coordinator::Coordinator>();
    coordinator->init();
    coordinator->add_event_listener(EVENT_FUNCTION_LISTENER(engine::event::QUIT, quit_handler));
    coordinator->add_event_listener(EVENT_FUNCTION_LISTENER(engine::event::RESIZE, resize_handler));

    window_manager = std::make_unique<desktop::window::WindowManager>();
    window_manager->initialize("App", 1280, 720);

    auto native_window = window_manager->get_native_window();

    graphics_manager = std::make_unique<engine::graphics::GraphicsManager>();
	graphics_manager->initialize(&native_window);

    window_manager->send_default_events();

    /// Register components

    coordinator->register_component<engine::component::Transform>();
	coordinator->register_component<engine::component::Camera>();
    coordinator->register_component<engine::component::RigidBody>();
    coordinator->register_component<engine::component::Gravity>();

    /*
    /// Systems

    auto physics_system = coordinator->register_system<engine::system::PhysicsSystem>();
	{
		engine::ecs::ECSMask mask;
        mask.set(coordinator->get_component_type<engine::component::Transform>());
		mask.set(coordinator->get_component_type<engine::component::Gravity>());
		mask.set(coordinator->get_component_type<engine::component::RigidBody>());
		coordinator->set_system_mask<engine::system::PhysicsSystem>(mask);
	}

    auto camera_control_system = coordinator->register_system<engine::system::CameraControlSystem>();
	{
		engine::ecs::ECSMask mask;
        mask.set(coordinator->get_component_type<engine::component::Transform>());
		mask.set(coordinator->get_component_type<engine::component::Camera>());
        //mask.set(coordinator.get_component_type<engine::component::RigidBody>());
		coordinator->set_system_mask<engine::system::CameraControlSystem>(mask);
	}

	camera_control_system->init();
    */

    //double last_time_fps = glfwGetTime();
    double last_time_frame = glfwGetTime();

    int nb_frames = 0;

    Diligent::float4x4 view = Diligent::float4x4::Translation(-Diligent::float3(0, 0, -5));

    while (!quit)
    {
        double current_time = glfwGetTime();

        /*
        double dt_fps = current_time - last_time_fps;

        if (dt_fps >= 1)
        {
            double fps = double(nbFrames) / delta;
            double duration = 1000.0 / double(nb_frames);

            std::stringstream ss;
            ss << std::setprecision(3) << fps << " fps / " << duration << "ms/frame";

            window_manager.update_title(ss.str().c_str());

            last_time_fps += 1.0;
            nb_frames = 0;
        }
        */

        double dt_frame = current_time - last_time_frame;

        window_manager->process_events();
        //camera_control_system->update(dt_frame);
        //physics_system->update(dt_frame);

        //view = camera_control_system->look_at();
        graphics_manager->set_camera_view(view);
        graphics_manager->update(dt_frame);

        last_time_frame = current_time;
        nb_frames++;
    }

    graphics_manager->shutdown();
    window_manager->shutdown();

    return EXIT_SUCCESS;
}
