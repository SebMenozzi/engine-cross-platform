set(SOURCE
    ${CMAKE_CURRENT_LIST_DIR}/camera_control_system.cpp
    ${CMAKE_CURRENT_LIST_DIR}/physics_system.cpp
)

target_sources(engine PRIVATE ${SOURCE})