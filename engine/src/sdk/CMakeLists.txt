add_library(Engine SHARED)

set_target_properties(Engine PROPERTIES
    FRAMEWORK TRUE
    PUBLIC_HEADER "${INCLUDE}"
    MACOSX_FRAMEWORK_INFO_PLIST ${CMAKE_CURRENT_LIST_DIR}/Info.plist
    XCODE_GENERATE_SCHEME TRUE
    XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "co.seb.engine-lib"
    XCODE_ATTRIBUTE_DEFINES_MODULE "YES"
    XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC "YES"
    XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++14"
    XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++"
    XCODE_ATTRIBUTE_INSTALL_PATH "@rpath"
    XCODE_ATTRIBUTE_SYMROOT "${PROJECT_BINARY_DIR}"
    # Generate dsym for all build types to ensure symbols available in profiling
    XCODE_ATTRIBUTE_GCC_GENERATE_DEBUGGING_SYMBOLS "YES"
)

target_link_libraries(Engine
    PUBLIC
    engine
    "-framework CoreGraphics"
    "-framework CoreLocation"
    "-framework QuartzCore"
    "-framework Metal"
)