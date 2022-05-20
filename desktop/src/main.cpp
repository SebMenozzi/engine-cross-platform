#include <iostream>

#include "engine.hpp"
#include "window_manager.hpp"

#include "utils_maths.hpp"
#include "event_types.hpp"

std::shared_ptr<desktop::window::WindowManager> window_manager = {};
std::shared_ptr<engine::Engine> engine_manager = {};

int main(int argc, char *argv[])
{
    window_manager = std::make_unique<desktop::window::WindowManager>();
    window_manager->initialize("Loading...", 1280, 720);

    auto native_window = window_manager->get_native_window();

    engine_manager = std::make_unique<engine::Engine>();
    engine_manager->init(native_window);

    window_manager->send_default_events();

    double last_time_fps = glfwGetTime();
    double last_time_frame = glfwGetTime();

    int nb_frames = 0;

    while (!engine_manager->should_quit())
    {
        double current_time = glfwGetTime();

        double dt_fps = current_time - last_time_fps;

        if (dt_fps >= 1)
        {
            double fps = double(nb_frames) / dt_fps;
            double duration = 1000.0 / double(nb_frames);

            std::stringstream ss;
            ss << std::setprecision(3) << fps << "fps / " << duration << "ms";

            window_manager->update_title(ss.str().c_str());

            last_time_fps = current_time;
            nb_frames = 0;
        }

        double dt = current_time - last_time_frame;

        window_manager->process_events();

        // There is no autorelease pool when this method is called
        // because it will be called from a background thread.
        // It's important to create one or app can leak objects.
        #if PLATFORM_MACOS
        @autoreleasepool {
        #endif
        if (window_manager->should_update())
            engine_manager->update(dt);
        #if PLATFORM_MACOS
        } // @autoreleasepool END
        #endif

        last_time_frame = current_time;
        nb_frames++;
    }

    engine_manager->shutdown();
    window_manager->shutdown();

    return EXIT_SUCCESS;
}
