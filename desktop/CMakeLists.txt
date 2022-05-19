add_executable(desktop)

target_sources(desktop PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/main.cpp
)

target_include_directories(desktop PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_LIST_DIR}/src/window
)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/src/window)

target_link_libraries(desktop PRIVATE
    Engine
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
        )

        # Only Xcode can process the xcassets bundle, so ignore it for other generators.
        set_target_properties(desktop
            PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Info.plist"
            RESOURCE "${RESOURCES}"
            XCODE_GENERATE_SCHEME TRUE
            XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC NO
            XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "co.seb.engine-app"
            XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME "AppIcon"
            XCODE_ATTRIBUTE_CODE_SIGN_STYLE "${CODE_SIGN_STYLE}"
            XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "${DEVELOPMENT_TEAM}"
            XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "${CODE_SIGN_IDENTITY}"
            CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "${CODE_SIGNING_REQUIRED}"
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