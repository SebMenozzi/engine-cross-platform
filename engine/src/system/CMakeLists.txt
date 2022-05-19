set(CMAKE_FOLDER "system")
set(MODULE system)

engine_library(${MODULE}
    camera_control_system.cpp
    camera_control_system.hpp
    physics_system.cpp
    physics_system.hpp
)

engine_link_libraries(${MODULE}
    ecs
    event
    utils
    component
    coordinator
)