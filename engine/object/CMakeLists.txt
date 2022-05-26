set(MODULE object)

engine_library(${MODULE}
    object.cpp
    object.hpp
    chunk.hpp
    cube.hpp
    plane.hpp
    sphere/uv_sphere.cpp
    sphere/uv_sphere.hpp
)

engine_link_libraries(${MODULE}
    utils
    diligent
)