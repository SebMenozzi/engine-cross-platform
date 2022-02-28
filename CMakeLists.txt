cmake_minimum_required(VERSION 3.15)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Generate XCode schema files
set(CMAKE_XCODE_GENERATE_SCHEME TRUE)
# Make malloc write 0xAA to newly allocated memory and 0x55 to deallocated memory
set(CMAKE_XCODE_SCHEME_MALLOC_SCRIBBLE YES)
# Place guard pages on each side of large (4096 bytes or more) buffers
set(CMAKE_XCODE_SCHEME_MALLOC_GUARD_EDGES YES)

project(engine)

add_subdirectory(submodules)

include_directories(
    src
)

# Default compilation flags
set(CMAKE_CXX_FLAGS "-pedantic -Wall -Wextra -fsanitize=address -Wno-deprecated-declarations -Wno-missing-field-initializers -Wno-unused-function -Wno-unused-parameter -Wno-switch -fvisibility=hidden")

# Debug compilation flags
set(CMAKE_CXX_FLAGS_DEBUG "-g3 -O0 -DDEBUG")

# Release compilation flags
set(CMAKE_CXX_FLAGS_RELEASE "-Ofast -DRELEASE")

# Exec
add_executable(engine)
target_sources(engine PRIVATE "src/main.cpp" ${SRC_CPP})

target_link_libraries(engine PRIVATE DiligentCore DiligentCorePro DiligentTools)