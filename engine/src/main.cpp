#include <iostream>
#include <Timer.hpp>

#include "ecs/ecs_types.hpp"
#include "graphics_manager.hpp"
#include "window_manager.hpp"
#include "utils_maths.hpp"
#include "coordinator.hpp"
#include "event_types.hpp"
#include "camera_control_system.hpp"

engine::Coordinator coordinator;
engine::window::WindowManager window_manager;
engine::graphics::GraphicsManager graphics_manager;

static bool quit = false;
static double max_framerate = 120.0;

void quit_handler(engine::event::Event& event)
{
	quit = true;
}

void resize_handler(engine::event::Event& event)
{
    int width = event.get_parameter<int>(engine::event::resize::WIDTH);
	int height = event.get_parameter<int>(engine::event::resize::HEIGHT);

	graphics_manager.resize(width, height);
}

int main(int argc, char *argv[])
{
    coordinator.init();
    coordinator.add_event_listener(EVENT_FUNCTION_LISTENER(engine::event::QUIT, quit_handler));
    coordinator.add_event_listener(EVENT_FUNCTION_LISTENER(engine::event::RESIZE, resize_handler));

    window_manager.initialize("App", 1280, 720);

    auto native_window = window_manager.get_native_window();
	graphics_manager.initialize(&native_window);

    window_manager.send_default_events();

    /// Register components

    coordinator.register_component<engine::component::Transform>();
	coordinator.register_component<engine::component::Camera>();

    /// Systems

    auto camera_control_system = coordinator.register_system<engine::system::CameraControlSystem>();
	{
		engine::ecs::ECSMask mask;
		mask.set(coordinator.get_component_type<engine::component::Camera>());
		mask.set(coordinator.get_component_type<engine::component::Transform>());
		coordinator.set_system_mask<engine::system::CameraControlSystem>(mask);
	}
	camera_control_system->init();

    auto timer = Diligent::Timer();
    double last_time = timer.GetElapsedTime();

    Diligent::float4x4 view;

    while (!quit)
    {
        double current_time = timer.GetElapsedTime();
        double dt = current_time - last_time;

        window_manager.process_events();
        camera_control_system->update(dt);

        if (dt >= 1.0 / max_framerate)
        {
            last_time = current_time;

            view = camera_control_system->look_at();
            graphics_manager.set_camera_view(view);
            graphics_manager.update(dt);
        }
    }

    graphics_manager.shutdown();
    window_manager.shutdown();

    return EXIT_SUCCESS;
}
