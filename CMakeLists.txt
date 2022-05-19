cmake_minimum_required(VERSION 3.19)

project(engine)

# Generate XCode schema files
set(CMAKE_XCODE_GENERATE_SCHEME TRUE)
# Make malloc write 0xAA to newly allocated memory and 0x55 to deallocated memory
set(CMAKE_XCODE_SCHEME_MALLOC_SCRIBBLE YES)
# Place guard pages on each side of large (4096 bytes or more) buffers
set(CMAKE_XCODE_SCHEME_MALLOC_GUARD_EDGES YES)

### Compiler
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "-Wno-deprecated-declarations -Wno-missing-field-initializers -Wno-unused-function -Wno-unused-parameter -Wno-switch")

### Set path to find cmake files
set(CMAKE_MODULE_PATH
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/"
    ${CMAKE_MODULE_PATH}
)
### Include cmake/engine.cmake
include(engine)

add_subdirectory(submodules)
add_subdirectory(engine)
add_subdirectory(desktop)
#add_subdirectory(iosapp)

### Create Diligent library

engine_library(diligent)

get_supported_backends(ENGINE_LIBRARIES)

engine_link_libraries(diligent
    Diligent-Common
    Diligent-GraphicsTools
    Diligent-BuildSettings
    Diligent-TargetPlatform
    Diligent-GraphicsAccessories
    Diligent-TextureLoader
    ${ENGINE_LIBRARIES}
)
