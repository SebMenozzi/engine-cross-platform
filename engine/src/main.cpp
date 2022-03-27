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
#include "graphics_engine.hpp"

static int window_width = 1280;
static int window_height = 720;

static GLFWwindow *window = nullptr;

static bool framebuffer_size_changed = false;
static bool cursor_disabled = false;

static std::shared_ptr<Diligent::GraphicsEngine> engine = {};

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
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                else
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

                cursor_disabled = !cursor_disabled;
            }
        default:
            break;
    }
}

static void framebuffer_resize_callback(GLFWwindow *window, int fWidth, int fHeight)
{
    // Indicate that the frame buffer size has changed for the next frame
    framebuffer_size_changed = true;
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

    // Create new instance of the engine
    engine = std::make_unique<Diligent::GraphicsEngine>();

    Diligent::NativeWindow native_window = get_native_window();
    engine->initialize(&native_window);

     // Setup the window's properties
    #ifdef PLATFORM_MACOS
        glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_TRUE);
    #endif
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    glfwSetWindowCloseCallback(window, window_close_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_resize_callback);

    int32_t width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    engine->resize(width, height);
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

        // If the framebuffer size has changed, let's resize the app
        if (framebuffer_size_changed)
        {
            framebuffer_size_changed = false;

            // Get the new framebuffer size
            int32_t width = 0, height = 0;
            glfwGetFramebufferSize(window, &width, &height);
            engine->resize(width, height);
        }

        engine->update();
    }
    
    engine->stop();

    glfwTerminate();

    return EXIT_SUCCESS;
}
