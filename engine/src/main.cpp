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

static bool cursor_disabled = false;

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
    graphics_engine->resize(fWidth, fHeight);
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
    graphics_engine = std::make_unique<engine::GraphicsEngine>();

    Diligent::NativeWindow native_window = get_native_window();
    graphics_engine->initialize(&native_window);

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
    graphics_engine->resize(width, height);
}

#ifdef PLATFORM_LINUX
    constexpr int kMaxFd = 32767;
#else
    constexpr int kMaxFd = 16384;
#endif

#ifdef PLATFORM_LINUX
    constexpr int kMaxMemlock = 16 * 1024 * 1024;
#else
    constexpr int kMaxMemlock = 65535;
#endif

void set_os_limit(int val, int feature)
{
    [[maybe_unused]] const char* label = [&] {
        switch (feature) {
            // This specifies a value one greater than the maximum file
            // descriptor number that can be opened by this process.
            case RLIMIT_NOFILE:
                return "max open files";
            // This is the maximum number of bytes of memory that may be
            // locked into RAM.
            case RLIMIT_MEMLOCK:
                return "max locked memory";
            default:
                return "unknown";
        }
    }();

    rlimit rlp;
    int result = getrlimit(feature, &rlp);
    if (result != 0)
    {
        [[maybe_unused]] int err = errno;
        std::cerr << "Failed to get  " << label << " limit: " << err << std::endl;
        return;
    }

    std::cout << "Current value for " << label << ": " << rlp.rlim_cur << std::endl;

    rlp.rlim_cur = val;
    rlp.rlim_max = val;

    if (setrlimit(feature, &rlp) != 0)
    {
        [[maybe_unused]] int err = errno;
        std::cerr << "Failed to set " << label << ". Errno: " << std::endl;
    } 
    else
    {
        std::cout << label << " set to " << val << std::endl;
    }
}

int main(int argc, char *argv[])
{

    set_os_limit(kMaxFd, RLIMIT_NOFILE);
    set_os_limit(kMaxMemlock, RLIMIT_MEMLOCK);

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
