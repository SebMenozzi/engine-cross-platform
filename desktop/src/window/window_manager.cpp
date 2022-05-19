#include "window_manager.hpp"
#include "GLFW/glfw3.h"

extern std::shared_ptr<engine::coordinator::Coordinator> coordinator;

namespace desktop
{
    namespace window
    {
        /// MARK: - Callbacks

        static void window_close_callback_(GLFWwindow *w)
        {
            coordinator->send_event(engine::event::QUIT);
        }

        static void key_callback_(GLFWwindow *w, int key, int scancode, int action, int mods)
        {
            auto window_manager = static_cast<WindowManager*>(glfwGetWindowUserPointer(w));

            switch (key) {
                case GLFW_KEY_ESCAPE:
                    if (action == GLFW_PRESS)
                    {
                        if (!window_manager->is_fullscreen())
                            coordinator->send_event(engine::event::QUIT);
                    }
                    break;
                case GLFW_KEY_C:
                    if (action == GLFW_PRESS)
                    {
                        if (window_manager->cursor_disabled)
                            glfwSetInputMode(w, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                        else
                            glfwSetInputMode(w, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

                        window_manager->cursor_disabled = !window_manager->cursor_disabled;
                    }
                    break;
                // Forward
                case GLFW_KEY_UP: case GLFW_KEY_W:
                    if (action == GLFW_PRESS)
                        window_manager->input.forward = true;
                    else if (action == GLFW_RELEASE)
                        window_manager->input.forward = false;
                    break;
                // Backward
                case GLFW_KEY_DOWN: case GLFW_KEY_S:
                    if (action == GLFW_PRESS)
                        window_manager->input.backward = true;
                    else if (action == GLFW_RELEASE)
                        window_manager->input.backward = false;
                    break;
                // Left
                case GLFW_KEY_LEFT: case GLFW_KEY_A:
                    if (action == GLFW_PRESS)
                        window_manager->input.left = true;
                    else if (action == GLFW_RELEASE)
                        window_manager->input.left = false;
                    break;
                // Right
                case GLFW_KEY_RIGHT: case GLFW_KEY_D:
                    if (action == GLFW_PRESS)
                        window_manager->input.right = true;
                    else if (action == GLFW_RELEASE)
                        window_manager->input.right = false;
                    break;
                // Up
                case GLFW_KEY_SPACE:
                    if (action == GLFW_PRESS)
                        window_manager->input.up = true;
                    else if (action == GLFW_RELEASE)
                        window_manager->input.up = false;
                    break;
                // Speed Up
                case GLFW_KEY_LEFT_SHIFT: case GLFW_KEY_RIGHT_SHIFT:
                    if (action == GLFW_PRESS)
                        window_manager->input.speed_up = true;
                    else if (action == GLFW_RELEASE)
                        window_manager->input.speed_up = false;
                    break;
                // Gravity
                case GLFW_KEY_G:
                    if (action == GLFW_PRESS)
                        window_manager->input.gravity = true;
                    else if (action == GLFW_RELEASE)
                        window_manager->input.gravity = false;
                    break;

                default:
                    break;
            }

            engine::event::Event input_event(engine::event::INPUT);
            input_event.set_parameter(engine::event::input::PARAMETER, window_manager->input);
            coordinator->send_event(input_event);
        }

        static void framebuffer_size_callback_(GLFWwindow *w, int width, int height)
        {
            engine::event::Event resize_event(engine::event::RESIZE);
		    resize_event.set_parameter(engine::event::resize::WIDTH, width);
            resize_event.set_parameter(engine::event::resize::HEIGHT, height);
            coordinator->send_event(resize_event);
        }

        static void mouse_callback_(GLFWwindow *w, double x, double y)
        {
            engine::event::Event mouse_position_event(engine::event::MOUSE_POSITION);
            mouse_position_event.set_parameter(engine::event::mouse_position::X, x);
            mouse_position_event.set_parameter(engine::event::mouse_position::Y, y);
            coordinator->send_event(mouse_position_event);
        }

        void scroll_callback_(GLFWwindow* w, double xoffset, double yoffset)
        {
            //fov -= yoffset;
            //fov = utils::clamp(fov, 45.0, 90.0);
            //graphics_engine->set_fov(fov);
        }

        /// MARK: - Public methods

        void WindowManager::initialize(
            std::string const& title,
            int width, 
            int height
        )
        {
            glfwInit();

            // Remove weird borders around the window on macos
            glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

            window_ = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
            if (!window_)
                return glfwTerminate();

            glfwSetWindowUserPointer(window_, this);

            // Cursor disabled by default
            glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

            if (glfwRawMouseMotionSupported())
                glfwSetInputMode(window_, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

            // Setup the window's properties
            #ifdef PLATFORM_MACOS
                glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_TRUE);
            #endif
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
            glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);

            glfwSetWindowCloseCallback(window_, window_close_callback_);
            glfwSetKeyCallback(window_, key_callback_);
            glfwSetFramebufferSizeCallback(window_, framebuffer_size_callback_);
            glfwSetCursorPosCallback(window_, mouse_callback_);
            glfwSetScrollCallback(window_, scroll_callback_);
        }

        void WindowManager::send_default_events()
        {
            int width = 0, height = 0;
            glfwGetFramebufferSize(window_, &width, &height);

            engine::event::Event resize_event(engine::event::RESIZE);
		    resize_event.set_parameter(engine::event::resize::WIDTH, width);
            resize_event.set_parameter(engine::event::resize::HEIGHT, height);
            coordinator->send_event(resize_event);

            double x, y;
            glfwGetCursorPos(window_, &x, &y);

            engine::event::Event mouse_position_event(engine::event::MOUSE_POSITION);
            mouse_position_event.set_parameter(engine::event::mouse_position::X, x);
            mouse_position_event.set_parameter(engine::event::mouse_position::Y, y);
            coordinator->send_event(mouse_position_event);
        }

        void WindowManager::process_events()
        {
            glfwPollEvents();
        }

        void WindowManager::shutdown()
        {
            assert(window_);

            glfwDestroyWindow(window_);
	        glfwTerminate();
        }

        Diligent::NativeWindow WindowManager::get_native_window()
        {
            assert(window_);

            Diligent::NativeWindow native_window;
            #ifdef PLATFORM_MACOS
                native_window.pNSView = make_native_metal_view(glfwGetCocoaWindow(window_));
            #endif
            #ifdef PLATFORM_LINUX
                native_window.WindowId = glfwGetX11Window(window_);
                native_window.pDisplay = glfwGetX11Display();
            #endif
            #ifdef PLATFORM_WIN32
                native_window.hWnd = glfwGetWin32Window(window_);
            #endif

            return native_window;
        }

        void WindowManager::update_title(std::string const& title)
        {
            glfwSetWindowTitle(window_, title.c_str());
        }

        bool WindowManager::is_fullscreen()
        {
            return glfwGetWindowMonitor(window_) != nullptr;
        }
    }
}