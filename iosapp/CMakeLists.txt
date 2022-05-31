# FIXME: CMake fails to detect a working Swift compiler. This just skips the check.
set(CMAKE_Swift_COMPILER_WORKS 1)
enable_language(Swift)

# Turn on ARC
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fobjc-arc")

set(PRODUCT_NAME ${APP_NAME})
set(EXECUTABLE_NAME ${APP_NAME})
set(MACOSX_BUNDLE_EXECUTABLE_NAME ${APP_NAME})
set(MACOSX_BUNDLE_INFO_STRING ${APP_BUNDLE_IDENTIFIER})
set(MACOSX_BUNDLE_GUI_IDENTIFIER ${APP_BUNDLE_IDENTIFIER})
set(MACOSX_BUNDLE_BUNDLE_NAME ${APP_BUNDLE_IDENTIFIER})
set(MACOSX_BUNDLE_ICON_FILE "")
set(MACOSX_BUNDLE_LONG_VERSION_STRING "1.0")
set(MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0")
set(MACOSX_BUNDLE_BUNDLE_VERSION "1.0")
set(MACOSX_BUNDLE_COPYRIGHT "Copyright Sebastien Menozzi")
set(MACOSX_DEPLOYMENT_TARGET ${DEPLOYMENT_TARGET})

set(APP_HEADER_FILES
	${CMAKE_CURRENT_LIST_DIR}/Sources/Bridging-Header.h
)

set(APP_SOURCE_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/Sources/AppDelegate.swift
	${CMAKE_CURRENT_SOURCE_DIR}/Sources/ViewController.swift
	${CMAKE_CURRENT_SOURCE_DIR}/Sources/CustomGestureRecognizer.swift
	${CMAKE_CURRENT_SOURCE_DIR}/Sources/Float+Extensions.swift
)

set(RESOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/Resources/Assets/AppIcon.xcassets
	${CMAKE_CURRENT_SOURCE_DIR}/Resources/Storyboards/LaunchScreen.storyboard
	${SHADERS}
	${TEXTURES}
)

add_executable(
    ${APP_NAME}
    MACOSX_BUNDLE
    ${APP_HEADER_FILES}
    ${APP_SOURCE_FILES}
    ${RESOURCES}
)

# Build the C++ dynamically linked framework
add_dependencies(${APP_NAME} ${FRAMEWORK_NAME})

#target_link_libraries(${APP_NAME} PRIVATE ${FRAMEWORK_NAME})

# Link the framework to the app
set_target_properties(${APP_NAME} PROPERTIES
    XCODE_ATTRIBUTE_OTHER_LDFLAGS "${XCODE_ATTRIBUTE_OTHER_LDFLAGS} -framework ${FRAMEWORK_NAME}"
)

# Create the app target
set_target_properties(${APP_NAME} PROPERTIES
	XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf-with-dsym"
	MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Info.plist"
	RESOURCE "${RESOURCES}"
	XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "${DEPLOYMENT_TARGET}"
	XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "Apple Development"
	XCODE_ATTRIBUTE_CODE_SIGN_STYLE "${CODE_SIGN_STYLE}"
	XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "${DEVELOPMENT_TEAM_ID}"
	XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "${DEVICE_FAMILY}"

	XCODE_ATTRIBUTE_SWIFT_OBJC_BRIDGING_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/Sources/Bridging-Header.h"
	XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME "AppIcon"

	XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC YES
	XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "${APP_BUNDLE_IDENTIFIER}"

	XCODE_ATTRIBUTE_INSTALL_PATH "$(LOCAL_APPS_DIR)"
	XCODE_ATTRIBUTE_GCC_SYMBOLS_PRIVATE_EXTERN YES
)

target_link_libraries(${APP_NAME}
	PUBLIC
	"-framework UIKit"
	"-framework CoreGraphics"
	"-framework QuartzCore"
	"-framework Metal"
)

# Include framework headers, needed to make "Build" Xcode action work.
# "Archive" works fine just relying on default search paths as it has different
# build product output directory.
target_include_directories(${APP_NAME} PUBLIC 
    "${PROJECT_BINARY_DIR}/framework/\${CONFIGURATION}\${EFFECTIVE_PLATFORM_NAME}/${FRAMEWORK_NAME}.framework"
)

# Set the app's linker search path to the default location on iOS
set_target_properties(
    ${APP_NAME}
    PROPERTIES
    XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS
    "@executable_path/Frameworks"
)

# Note that commands below are indented just for readability. They will endup as
# one liners after processing and unescaped ; will disappear so \; are needed.
# First condition in each command is for normal build, second for archive.
# \&\>/dev/null makes sure that failure of one command and success of other
# is not printed and does not make Xcode complain that /bin/sh failed and build
# continued.

# Create Frameworks directory in app bundle
add_custom_command(
    TARGET
    ${APP_NAME}
    POST_BUILD COMMAND /bin/sh -c
    \"
	echo CREATE FRAMEWORKS DIRECTORY \;

	${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/\${CONFIGURATION}\${EFFECTIVE_PLATFORM_NAME}/${APP_NAME}.app/Frameworks \;
    \"
)

# Copy the framework into the app bundle
add_custom_command(
    TARGET
    ${APP_NAME}
    POST_BUILD COMMAND /bin/sh -c
    \"
	echo COPY FRAMEWORK \;

	${CMAKE_COMMAND} -E copy_directory 
		${PROJECT_BINARY_DIR}/framework/\${CONFIGURATION}\${EFFECTIVE_PLATFORM_NAME}/
		${PROJECT_BINARY_DIR}/iosapp/\${CONFIGURATION}\${EFFECTIVE_PLATFORM_NAME}/${APP_NAME}.app/Frameworks \;
    \"
)

# Copy the framework into the app bundle
add_custom_command(
    TARGET
    ${APP_NAME}
    POST_BUILD COMMAND /bin/sh -c
    \"
	echo COPY FRAMEWORK \;

	${CMAKE_COMMAND} -E copy_directory 
		${PROJECT_BINARY_DIR}/framework/\${CONFIGURATION}\${EFFECTIVE_PLATFORM_NAME}/
		${PROJECT_BINARY_DIR}/iosapp/\${CONFIGURATION}\${EFFECTIVE_PLATFORM_NAME}/${APP_NAME}.app/Frameworks \;
    \"
)

# Codesign the framework in it's new spot
add_custom_command(
    TARGET
    ${APP_NAME}
    POST_BUILD COMMAND /bin/sh -c
    \"
	echo CODESIGN FRAMEWORK \;

	codesign --force --verbose 
		${PROJECT_BINARY_DIR}/iosapp/\${CONFIGURATION}\${EFFECTIVE_PLATFORM_NAME}/${APP_NAME}.app/Frameworks/${FRAMEWORK_NAME}.framework 
		--sign ${CODE_SIGN_IDENTITY} \;
	\"
)

# To appear in the IDE
source_group("Shaders" FILES ${SHADERS})
source_group("Textures" FILES ${TEXTURES})