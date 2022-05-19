set(CMAKE_FOLDER "component")
set(MODULE component)

engine_library(${MODULE}
    camera.hpp
    gravity.hpp
    rigid_body.hpp
    transform.hpp
)

engine_link_libraries(${MODULE}
    diligent
)