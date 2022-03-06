add_library(Engine SHARED)

# --------------

set(SOURCE
    ${CMAKE_CURRENT_LIST_DIR}/src/GraphicsEngine.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/GraphicsEngineMacOSImplem.mm
)

set(INCLUDE
    ${CMAKE_CURRENT_LIST_DIR}/src/CrossPlatformGraphicsEngine.hpp
    ${CMAKE_CURRENT_LIST_DIR}/src/GraphicsEngine.hpp
    ${CMAKE_CURRENT_LIST_DIR}/src/GraphicsEngineInterface.hpp
    ${CMAKE_CURRENT_LIST_DIR}/src/GraphicsEngineMacOS.hpp
)

set(SHADERS
    ${CMAKE_CURRENT_LIST_DIR}/assets/shaders/Cube.vsh
    ${CMAKE_CURRENT_LIST_DIR}/assets/shaders/Cube.psh
)

set(ASSETS ${SHADERS})

# --------------

target_sources(Engine PRIVATE ${INCLUDE} ${SOURCE} ${SHADERS})

# --------------

set_target_properties(Engine PROPERTIES
    FRAMEWORK TRUE
    PUBLIC_HEADER "${INCLUDE}"
    MACOSX_FRAMEWORK_INFO_PLIST ${CMAKE_CURRENT_LIST_DIR}/Info.plist
    XCODE_GENERATE_SCHEME TRUE
    XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "co.seb.engine-lib"
    XCODE_ATTRIBUTE_DEFINES_MODULE "YES"
    XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC "YES"
    XCODE_ATTRIBUTE_INSTALL_PATH "@rpath"
    XCODE_ATTRIBUTE_SYMROOT "${PROJECT_BINARY_DIR}"
    # Generate dsym for all build types to ensure symbols available in profiling
    XCODE_ATTRIBUTE_GCC_GENERATE_DEBUGGING_SYMBOLS "YES"
)

# --------------

target_link_libraries(Engine
    PUBLIC
    "-framework CoreGraphics"
    "-framework CoreLocation"
    "-framework QuartzCore"
    "-framework Metal"
)

# --------------

target_include_directories(Engine PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_LIST_DIR}/assets
    ${CMAKE_CURRENT_LIST_DIR}/assets/shaders
)

get_supported_backends(ENGINE_LIBRARIES)

target_link_libraries(Engine PRIVATE
    Diligent-Common
    Diligent-GraphicsTools
    Diligent-BuildSettings
    Diligent-TargetPlatform
    Diligent-GraphicsAccessories
    Diligent-TextureLoader
    Diligent-RenderStateNotation
    DiligentFX
    Diligent-Imgui
    ${ENGINE_LIBRARIES}
)

# --------------

source_group("Sources" FILES ${SOURCE} ${INCLUDE})
source_group("Assets" FILES ${ASSETS})
