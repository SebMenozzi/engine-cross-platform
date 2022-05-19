set(CMAKE_FOLDER "coordinator")
set(MODULE coordinator)

engine_library(${MODULE}
    coordinator.hpp
)

engine_link_libraries(${MODULE}
    ecs
    event
)