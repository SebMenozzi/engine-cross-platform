add_subdirectory(component)
add_subdirectory(coordinator)
add_subdirectory(ecs)
add_subdirectory(event)
add_subdirectory(graphics)
add_subdirectory(object)
add_subdirectory(system)
add_subdirectory(utils)

set(MODULE engine)

engine_library(${MODULE}
    engine.cpp
    engine.hpp
)

engine_link_libraries(${MODULE}
    PUBLIC
    component
    coordinator
    ecs
    event
    graphics
    object
    utils
    system
    diligent
)

if (PLATFORM_MACOS)
    # MacOS needs @autoreleasepool wrapped around the frame drawing to prevent
    # memory leaks.
    set_source_files_properties(engine.cpp COMPILE_FLAGS "-x objective-c++")
endif()