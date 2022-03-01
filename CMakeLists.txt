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
add_subdirectory(SampleBase)

# --------------

set(SOURCE
    src/AtmosphereSample.cpp
    src/Terrain/EarthHemisphere.cpp
    src/Terrain/ElevationDataSource.cpp
)

set(INCLUDE
    src/AtmosphereSample.hpp
    src/Terrain/DynamicQuadTreeNode.hpp
    src/Terrain/EarthHemisphere.hpp
    src/Terrain/ElevationDataSource.hpp
    src/Terrain/HierarchyArray.hpp
)

set(SHADERS
    assets/shaders/HostSharedTerrainStructs.fxh
    assets/shaders/EpipolarLightScatteringStructures.fxh
    assets/shaders/EpipolarLightScatteringFunctions.fxh
    assets/shaders/ToneMappingStructures.fxh
    assets/shaders/BasicStructures.fxh
    assets/shaders/Shadows.fxh
    assets/shaders/terrain/GenerateNormalMapPS.fx
    assets/shaders/terrain/HemispherePS.fx
    assets/shaders/terrain/HemisphereVS.fx
    assets/shaders/terrain/HemisphereZOnlyVS.fx
    assets/shaders/terrain/ScreenSizeQuadVS.fx
    assets/shaders/terrain/TerrainShadersCommon.fxh
)

set(RENDER_STATES
    assets/RenderStates.json
)

set(TERRAIN_TEXTURES
    assets/terrain/heightmap.tif
    assets/terrain/mask.png
)

set(TERRAIN_TILES
    assets/terrain/tiles/cliff_DM.dds
    assets/terrain/tiles/cliff_NM.dds
    assets/terrain/tiles/grass_DM.dds
    assets/terrain/tiles/grass_NM.dds
    assets/terrain/tiles/grassDark_DM.dds
    assets/terrain/tiles/gravel_DM.dds
    assets/terrain/tiles/gravel_NM.dds
    assets/terrain/tiles/snow_DM.dds
    assets/terrain/tiles/snow_NM.jpg
)

set(ASSETS ${RENDER_STATES} ${TERRAIN_TEXTURES} ${TERRAIN_TILES})

# --------------

set_source_files_properties(${SHADERS} PROPERTIES VS_TOOL_OVERRIDE "None")
set(ALL_ASSETS ${ASSETS} ${SHADERS})
add_target_platform_app(engine "${SOURCE}" "${INCLUDE}" "${ALL_ASSETS}")

set_source_files_properties(${ALL_ASSETS} PROPERTIES 
    VS_DEPLOYMENT_LOCATION "."
    MACOSX_PACKAGE_LOCATION "Resources" 
)

if(PLATFORM_WIN32)
    set_target_properties(engine PROPERTIES 
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/assets"
    )
    copy_required_dlls(engine)
    append_sample_base_win32_source(engine)
elseif(PLATFORM_UNIVERSAL_WINDOWS)
    append_sample_base_uwp_source(engine)
    package_required_dlls(engine)
endif()

target_include_directories(engine PRIVATE src)

target_link_libraries(engine PRIVATE
    # On Linux we must have Diligent-NativeAppBase go first, otherwise the linker 
    # will fail to resolve Diligent::CreateApplication() function.
    Diligent-NativeAppBase
    Diligent-BuildSettings
    Diligent-SampleBase
)
set_common_target_properties(engine)

if(MSVC)
    # Disable MSVC-specific warnings
    # - w4201: nonstandard extension used: nameless struct/union
    target_compile_options(engine PRIVATE /wd4201)
endif()

source_group("src" FILES ${SOURCE} ${INCLUDE})
source_group("assets" FILES ${ALL_ASSETS})	

target_sources(engine PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set_source_files_properties(
    "${CMAKE_CURRENT_SOURCE_DIR}/README.md" PROPERTIES HEADER_FILE_ONLY TRUE
)

if(PLATFORM_WIN32 OR PLATFORM_LINUX)
    # Copy assets to target folder
    add_custom_command(TARGET engine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/assets"
            "\"$<TARGET_FILE_DIR:${ECXE_NAME}>\""
    )
endif()

if(PLATFORM_MACOS AND VULKAN_LIB_PATH)
    # Configure rpath so that executables can find vulkan library
    set_target_properties(engine PROPERTIES
        BUILD_RPATH "${VULKAN_LIB_PATH}"
    )
endif()

# --------------

target_include_directories(engine PRIVATE
    src
    src/Terrain
    assets/shaders
    assets/shaders/terrain
)

target_link_libraries(engine PRIVATE
    Diligent-GraphicsAccessories
    Diligent-TextureLoader
    Diligent-RenderStateNotation
    DiligentFX
)