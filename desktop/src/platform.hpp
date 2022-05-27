#pragma once

#include <string>

#if PLATFORM_MACOS
    void *make_native_metal_view(void *native_window_handle);
    const std::string& get_resource_path();
#endif