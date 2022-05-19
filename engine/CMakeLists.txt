add_subdirectory(src)

set(SHADERS
    ${CMAKE_CURRENT_LIST_DIR}/assets/shaders/cube/cube.vsh
    ${CMAKE_CURRENT_LIST_DIR}/assets/shaders/cube/cube.psh
)

set(ASSETS ${SHADERS})

# --------------

source_group("Sources" FILES ${SOURCE} ${INCLUDE})
source_group("Assets" FILES ${ASSETS})