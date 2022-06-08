cmake_minimum_required(VERSION 3.20)

project(engine)

# Just to delete the warning CMAKE_TOOLCHAIN_FILE unused
# https://stackoverflow.com/questions/14757506/why-am-i-warned-that-the-cmake-toolchain-file-variable-is-not-used-by-the-proj
MESSAGE(STATUS "Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}")

# Generate xcode schema files
set(CMAKE_XCODE_GENERATE_SCHEME TRUE)
# Make malloc write 0xAA to newly allocated memory and 0x55 to deallocated memory
set(CMAKE_XCODE_SCHEME_MALLOC_SCRIBBLE YES)
# Place guard pages on each side of large (4096 bytes or more) buffers
set(CMAKE_XCODE_SCHEME_MALLOC_GUARD_EDGES YES)

### Compiler parameters
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# Removing one of these flags MIGHT make the project not building anymore
set(CMAKE_CXX_FLAGS "-Wno-deprecated-declarations -Wno-missing-field-initializers -Wno-unused-function -Wno-unused-parameter -Wno-switch -Wno-unused-const-variable -Wno-c++11-narrowing")

# Set path to find cmake files
set(CMAKE_MODULE_PATH
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/"
    ${CMAKE_MODULE_PATH}
)
# Include cmake/engine.cmake
include(engine)

add_subdirectory(submodules)
add_subdirectory(engine)

set(SHADERS
    ${PROJECT_SOURCE_DIR}/assets/shaders/structures.fxh
    ${PROJECT_SOURCE_DIR}/assets/shaders/post_process/post_process.psh
	${PROJECT_SOURCE_DIR}/assets/shaders/post_process/post_process.vsh
	${PROJECT_SOURCE_DIR}/assets/shaders/texture/texture.psh
	${PROJECT_SOURCE_DIR}/assets/shaders/texture/texture.vsh
	${PROJECT_SOURCE_DIR}/assets/shaders/plane/plane.psh
	${PROJECT_SOURCE_DIR}/assets/shaders/plane/plane.vsh
)

set(TEXTURES
	${PROJECT_SOURCE_DIR}/assets/dirt.jpg
	${PROJECT_SOURCE_DIR}/assets/mj.jpg
	${PROJECT_SOURCE_DIR}/assets/wood.jpeg
)

if (IOS_PLATFORM) # ON when using the iOS cmake toolchain
    # Set to your project name, e.g. project.xcodeproj
    set(DEVELOPMENT_PROJECT_NAME "iosapp")           
    # Set to your team ID from Apple
    set(DEVELOPMENT_TEAM_ID "2TE7AHNP9P")
    # Set To your app's name
    set(APP_NAME "iosapp")
    # Set to your app's bundle identifier
    set(APP_BUNDLE_IDENTIFIER "com.seb.iosapp")
    # Set to your preferred code sign identity, to see list:
    # => /usr/bin/env xcrun security find-identity -v -p codesigning
    set(CODE_SIGN_IDENTITY "8F4A1C337BDDBA2D1781AC7FFB3DEB4294FC32AF") 
    # Set to Manual or Automatic signing
    set(CODE_SIGN_STYLE "Automatic")
    # Set your deployment target version of iOS                 
    set(DEPLOYMENT_TARGET 15.4)
    # Set to "1" to target iPhone, set to "2" to target iPad, set to "1,2" to target both
    set(DEVICE_FAMILY "1")

    set(FRAMEWORK_NAME "Engine")
    set(FRAMEWORK_BUNDLE_IDENTIFIER "co.seb.engine") 

    add_subdirectory(framework)
    add_subdirectory(iosapp)
else()
    add_subdirectory(desktop)
endif()

### Create the Diligent library

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