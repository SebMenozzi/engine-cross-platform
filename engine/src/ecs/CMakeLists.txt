set(CMAKE_FOLDER "ecs")
set(MODULE ecs)

engine_library(${MODULE}
    ecs_component_array.hpp
    ecs_component_manager.hpp
    ecs_entity_manager.hpp
    ecs_system_manager.hpp
    ecs_system.hpp
    ecs_types.hpp
)