find_package(OpenGL REQUIRED)

# Diligent Core

set(DILIGENT_CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/DiligentCore)
add_subdirectory(${DILIGENT_CORE_DIR})

# Diligent Core Pro

set(DILIGENT_CORE_PRO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/DiligentCorePro)
add_subdirectory(${DILIGENT_CORE_PRO_DIR})

# Diligent Tools

set(DILIGENT_TOOLS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/DiligentTools)
add_subdirectory(${DILIGENT_TOOLS_DIR})

# GLFW

set(GLFW_DIR ${CMAKE_CURRENT_SOURCE_DIR}/glfw)
add_subdirectory(${GLFW_DIR})

add_library(GLFW_LIBRARY INTERFACE)
target_include_directories(GLFW_LIBRARY INTERFACE ${GLFW_DIR}/include)
target_link_libraries(GLFW_LIBRARY INTERFACE ${OPENGL_LIBRARIES} glfw)