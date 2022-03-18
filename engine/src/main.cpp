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
#include "GraphicsEngine.hpp"

static int window_width = 1280;
static int window_height = 720;

static GLFWwindow *window = nullptr;

static bool framebuffer_size_changed = false;
static bool focused = true;

static std::shared_ptr<Diligent::GraphicsEngine> engine = {};

static void window_close_callback(GLFWwindow *window)
{
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

static void scroll_callback(GLFWwindow *window, double scrollx, double scrolly)
{

}

static void cursor_pos_callback(GLFWwindow *window, double x, double y)
{

}

static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{

}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    // Ignore input if window should close
    if (glfwWindowShouldClose(window)) {
        return;
    }

    switch (key) {
        case GLFW_KEY_ESCAPE:
            if (action == GLFW_PRESS)
                window_close_callback(window);
            break;
        default:
            break;
    }
}

static void char_callback(GLFWwindow *window, unsigned int c)
{

}

static void framebuffer_resize_callback(GLFWwindow *window, int fWidth, int fHeight)
{
    // Indicate that the frame buffer size has changed for the next frame
    framebuffer_size_changed = true;
}

static void drop_callback(GLFWwindow *window, int count, const char **paths)
{

}

static void window_focused_callback(GLFWwindow *window, int new_focused)
{
    focused = new_focused;
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

static void error_callback(int code, const char *description)
{
    std::cerr << "GLFW error (code " << code << "): " << description << std::endl;
}

static void init_engine()
{
    glfwSetErrorCallback(error_callback);

    window = glfwCreateWindow(window_width, window_height, "App", nullptr, nullptr);
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

    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetWindowCloseCallback(window, window_close_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCharCallback(window, char_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_resize_callback);
    glfwSetDropCallback(window, drop_callback);
    glfwSetWindowFocusCallback(window, window_focused_callback);

    int32_t fWidth = 0, fHeight = 0, wWidth = 0, wHeight = 0;
    glfwGetFramebufferSize(window, &fWidth, &fHeight);
    glfwGetWindowSize(window, &wWidth, &wHeight);

    engine->resize(fWidth, fHeight);

    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
}

int main(int argc, char *argv[])
{
    glfwSetErrorCallback(error_callback);

    // Init glfw
    if (!glfwInit())
    {
        std::cerr << "Error while initializing GLFW3" << std::endl;
        return EXIT_FAILURE;
    }

    init_engine();

    #if PLATFORM_MACOS 
        // GLFW + CATALINA workaround. This fixes the empty window bug. See
        // https://github.com/glfw/glfw/issues/1334
        int x, y;
        glfwGetWindowPos(window, &x, &y);
        glfwSetWindowPos(window, ++x, y);
    #endif

    // Give it a chance to shutdown cleanly on CTRL-C
    signal(SIGINT, [](int) {
        if (!glfwWindowShouldClose(window)) {
            glfwSetWindowShouldClose(window, 1);
            glfwPostEmptyEvent();
        } else {
            exit(1);
        }
    });

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // If the framebuffer size has changed, let's resize the app
        if (framebuffer_size_changed) {
            framebuffer_size_changed = false;

            // Get the new framebuffer size
            int32_t width = 0, height = 0;
            glfwGetFramebufferSize(window, &width, &height);

            engine->resize(width, height);
        }

        engine->start();
    }
    
    engine->stop();

    glfwTerminate();

    return EXIT_SUCCESS;
}
