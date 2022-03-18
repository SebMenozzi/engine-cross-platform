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
set(CMAKE_CXX_FLAGS "-Wno-deprecated-declarations -Wno-missing-field-initializers -Wno-unused-function -Wno-unused-parameter -Wno-switch -fvisibility=hidden")

add_subdirectory(submodules)
add_subdirectory(engine)