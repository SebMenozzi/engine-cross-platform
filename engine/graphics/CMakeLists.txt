set(MODULE graphics)

engine_library(${MODULE}
    graphics_manager.cpp
    graphics_manager.hpp
    graphics_utils.cpp
    graphics_utils.hpp
)

engine_link_libraries(${MODULE}
    object
    utils
    diligent
)