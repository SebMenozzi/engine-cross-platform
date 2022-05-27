add_executable(desktop)

# Override resource assets path
if (NOT CMAKE_GENERATOR MATCHES "Xcode")
    file(COPY ${SHADERS} DESTINATION ${PROJECT_BINARY_DIR}/desktop)
    file(COPY ${TEXTURES} DESTINATION ${PROJECT_BINARY_DIR}/desktop)
endif()

target_sources(desktop PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/main.cpp
)

target_include_directories(desktop PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_LIST_DIR}/src/window
)

if (PLATFORM_MACOS)
    # MacOS needs @autoreleasepool wrapped around the frame drawing to prevent
    # memory leaks.
    set_source_files_properties(${CMAKE_CURRENT_LIST_DIR}/src/main.cpp COMPILE_FLAGS "-x objective-c++")
endif()

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/src/window)

target_link_libraries(desktop PRIVATE
    engine
    GLFW_LIBRARY
)

if (PLATFORM_MACOS)
    if (CMAKE_GENERATOR MATCHES "Xcode")
        # The Xcode scheme parameters below require CMake 3.15 or newer.
        cmake_minimum_required(VERSION 3.15)

        set(CODE_SIGN_STYLE "Manual")
        set(DEVELOPMENT_TEAM "") 
        set(CODE_SIGN_IDENTITY "")
        set(CODE_SIGNING_REQUIRED "NO")

        set(RESOURCES
            ${CMAKE_CURRENT_SOURCE_DIR}/Resources/AppIcon.xcassets
            ${SHADERS}
            ${TEXTURES}
        )

        # Only Xcode can process the xcassets bundle, so ignore it for other generators.
        set_target_properties(desktop
            PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Info.plist"
            RESOURCE "${RESOURCES}"
            XCODE_GENERATE_SCHEME TRUE
            XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC NO
            XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "co.seb.desktop"
            XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME "AppIcon"
            XCODE_ATTRIBUTE_CODE_SIGN_STYLE "${CODE_SIGN_STYLE}"
            XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "${DEVELOPMENT_TEAM}"
            XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "${CODE_SIGN_IDENTITY}"
            CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "${CODE_SIGNING_REQUIRED}"
            XCODE_ATTRIBUTE_SWIFT_OBJC_BRIDGING_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/Sources/Bridging-Header.h"
            XCODE_ATTRIBUTE_FRAMEWORK_SEARCH_PATHS "$(PROJECT_DIR)/build/${CMAKE_BUILD_TYPE}"
        )
        target_sources(desktop
            PRIVATE
            "$<TARGET_PROPERTY:RESOURCE>"
        )
    endif()

    target_sources(desktop
        PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src/platform.mm
    )

    target_link_libraries(desktop
        PRIVATE
        "-framework CoreGraphics"
        "-framework CoreLocation"
        "-framework QuartzCore"
        "-framework Metal"
        "-framework MetalKit"
        "-framework AppKit"
    )
endif()

# Group the files by folder in the IDE
source_group("Shaders" FILES ${SHADERS})
source_group("Textures" FILES ${TEXTURES})