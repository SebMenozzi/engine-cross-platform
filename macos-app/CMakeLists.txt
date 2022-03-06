add_executable(MacOSApp)

#FIXME: CMake fails to detect a working Swift compiler. This just skips the check.
set(CMAKE_Swift_COMPILER_WORKS 1)
enable_language(Swift)

set(RESOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Resources/AppIcon.xcassets
)

target_sources(MacOSApp
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/Sources/main.swift
  ${CMAKE_CURRENT_SOURCE_DIR}/Sources/AppDelegate.swift
  ${CMAKE_CURRENT_SOURCE_DIR}/Sources/Operator.swift
  ${CMAKE_CURRENT_LIST_DIR}/Sources/Bridging-Header.h
  ${CMAKE_CURRENT_LIST_DIR}/Sources/Wrapper.h
  ${CMAKE_CURRENT_LIST_DIR}/Sources/Wrapper.mm
  ${RESOURCES}
)

target_link_libraries(MacOSApp
  PUBLIC
  Engine
)

set(CODE_SIGN_STYLE "Manual")
set(DEVELOPMENT_TEAM "") 
set(CODE_SIGN_IDENTITY "")
set(CODE_SIGNING_REQUIRED "NO")

set_target_properties(MacOSApp
  PROPERTIES
  MACOSX_BUNDLE TRUE
  MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Info.plist"
  RESOURCE "${RESOURCES}"
  XCODE_GENERATE_SCHEME TRUE
  XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC YES
  XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "co.seb.engine-macos-app"
  XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME "AppIcon"
  XCODE_ATTRIBUTE_CODE_SIGN_STYLE "${CODE_SIGN_STYLE}"
  XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "${DEVELOPMENT_TEAM}"
  XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "${CODE_SIGN_IDENTITY}"
  CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "${CODE_SIGNING_REQUIRED}"
  XCODE_ATTRIBUTE_SWIFT_OBJC_BRIDGING_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/Sources/Bridging-Header.h"
  XCODE_ATTRIBUTE_FRAMEWORK_SEARCH_PATHS "$(PROJECT_DIR)/build/MacOS/${CMAKE_BUILD_TYPE}"
)

target_link_libraries(MacOSApp
    PUBLIC
    "-framework CoreGraphics"
    "-framework CoreLocation"
    "-framework QuartzCore"
    "-framework Metal"
)
