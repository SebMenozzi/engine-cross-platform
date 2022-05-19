set(CMAKE_FOLDER "event")
set(MODULE event)

engine_library(${MODULE}
    event_manager.hpp
    event_types.hpp
    event.hpp
)

engine_link_libraries(${MODULE}
    utils
)