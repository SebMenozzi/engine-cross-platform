#include <iostream>
#include <signal.h>

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

#include "platform.hpp"
#include "camera.hpp"
#include "graphics_engine.hpp"
#include "maths_utils.hpp"

static int window_width = 1280;
static int window_height = 720;

static bool is_speed_up = false;
static double fov = 45.0f;
static engine::camera::Direction move_direction;

static GLFWwindow *window = nullptr;

static bool cursor_disabled = true;

static std::shared_ptr<engine::camera::Camera> camera = {};
static std::shared_ptr<engine::GraphicsEngine> graphics_engine = {};

static void window_close_callback(GLFWwindow *window)
{
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    // Ignore input if window should close
    if (glfwWindowShouldClose(window))
        return;

    switch (key) {
        case GLFW_KEY_ESCAPE:
            if (action == GLFW_PRESS)
                window_close_callback(window);
            break;
        case GLFW_KEY_C:
            if (action == GLFW_PRESS)
            {
                if (cursor_disabled)
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                else
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

                cursor_disabled = !cursor_disabled;
            }
            break;
        // Forward
        case GLFW_KEY_UP: case GLFW_KEY_W:
            if (action == GLFW_PRESS)
                move_direction.forward = true;
            else if (action == GLFW_RELEASE)
                move_direction.forward = false;
            break;
        // Backward
        case GLFW_KEY_DOWN: case GLFW_KEY_S:
            if (action == GLFW_PRESS)
                move_direction.backward = true;
            else if (action == GLFW_RELEASE)
                move_direction.backward = false;
            break;
        // Left
        case GLFW_KEY_LEFT: case GLFW_KEY_A:
            if (action == GLFW_PRESS)
                move_direction.left = true;
            else if (action == GLFW_RELEASE)
                move_direction.left = false;
            break;
        // Right
        case GLFW_KEY_RIGHT: case GLFW_KEY_D:
            if (action == GLFW_PRESS)
                move_direction.right = true;
            else if (action == GLFW_RELEASE)
                move_direction.right = false;
            break;
        // Up
        case GLFW_KEY_SPACE:
            if (action == GLFW_PRESS)
                move_direction.up = true;
            else if (action == GLFW_RELEASE)
                move_direction.up = false;
            break;
        // Speed Up
        case GLFW_KEY_LEFT_SHIFT: case GLFW_KEY_RIGHT_SHIFT:
            if (action == GLFW_PRESS)
                is_speed_up = true;
            else if (action == GLFW_RELEASE)
                is_speed_up = false;
            break;
        // Enable Gravity
        case GLFW_KEY_G:
            if (action == GLFW_PRESS)
                camera->toggle_gravity();
            break;

        default:
            break;
    }
    
    camera->set_move_direction(move_direction);
    camera->set_is_speed_up(is_speed_up);
}

static void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    graphics_engine->resize(width, height);
}

static void mouse_callback(GLFWwindow *window, double x, double y)
{
    if (cursor_disabled)
        camera->set_mouse_position(Diligent::float2(x, y));
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    fov -= yoffset;
    fov = engine::utils::clamp(fov, 45.0, 90.0);
    graphics_engine->set_fov(fov);
}

static Diligent::NativeWindow get_native_window()
{
    Diligent::NativeWindow native_window;
    #ifdef PLATFORM_MACOS
        native_window.pNSView = make_native_metal_view(glfwGetCocoaWindow(window));
    #endif
    #ifdef PLATFORM_LINUX
        native_window.WindowId = glfwGetX11Window(window);
        native_window.pDisplay = glfwGetX11Display();
    #endif
    #ifdef PLATFORM_WIN32
        native_window.hWnd = glfwGetWin32Window(window);
    #endif

    return native_window;
}

static void init_engine()
{
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

    window = glfwCreateWindow(window_width, window_height, "App", NULL, NULL);
    if (!window)
        return glfwTerminate();

    // Cursor disabled by default
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (glfwRawMouseMotionSupported())
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    camera = std::make_unique<engine::camera::Camera>(
        Diligent::float3(0, 0, -5), // Position
        Diligent::float3(0, 1, 0), // Head is up (set to 0,-1,0 to look upside-down)
        0.1,
        0.05
    );
    graphics_engine = std::make_unique<engine::GraphicsEngine>(camera);

    Diligent::NativeWindow native_window = get_native_window();
    graphics_engine->initialize(&native_window);

     // Setup the window's properties
    #ifdef PLATFORM_MACOS
        glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_TRUE);
    #endif
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);

    glfwSetWindowCloseCallback(window, window_close_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    int32_t width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    graphics_engine->resize(width, height);

    double x, y;
    glfwGetCursorPos(window, &x, &y);
    camera->set_mouse_position(Diligent::float2(x, y));

    graphics_engine->set_fov(fov);
}

int main(int argc, char *argv[])
{
    // Init glfw
    if (!glfwInit())
    {
        std::cerr << "Error while initializing GLFW3" << std::endl;
        return EXIT_FAILURE;
    }

    init_engine();

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        graphics_engine->update();
    }
    
    graphics_engine->stop();

    glfwTerminate();

    return EXIT_SUCCESS;
}
