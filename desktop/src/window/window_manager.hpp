#pragma once

#ifdef PLATFORM_MACOS
    #define GLFW_EXPOSE_NATIVE_COCOA
#endif
#ifdef PLATFORM_LINUX
    #define GLFW_EXPOSE_NATIVE_X11
#endif
#ifdef PLATFORM_WIN32
    #define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <NativeWindow.h>

#include "engine.hpp"
#include "platform.hpp"
#include "event_types.hpp"
#include "utils_types.hpp"

namespace desktop
{
    namespace window
    {
        class WindowManager
        {
            public:
                void initialize(
                    std::string const& title, 
                    int width, 
                    int height
                );
                void send_default_events();
                void process_events();
                void shutdown();
                Diligent::NativeWindow get_native_window();
                void send_input(Input input);
                bool is_fullscreen();
                void update_title(std::string const& title);
        
                bool cursor_disabled = true;
                Input input;
            private:
                GLFWwindow* window_;
            };
    }
}