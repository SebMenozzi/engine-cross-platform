set(SOURCE
    ${CMAKE_CURRENT_LIST_DIR}/object.cpp
)

target_sources(engine PRIVATE ${SOURCE})
target_include_directories(engine PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/sphere
)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/sphere)