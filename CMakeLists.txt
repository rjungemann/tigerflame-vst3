cmake_minimum_required(VERSION 3.25)

# Project metadata
set(PROJECT_NAME tigerflame-vst3)
set(PROJECT_VERSION 0.1.0)
set(PROJECT_DESCRIPTION "VST3/CLAP plugin for mml2vgm-compatible sound chip instruments")
set(PROJECT_VENDOR "TigerFlame")
set(PROJECT_WEBSITE "https://github.com/rjungemann/tigerflame-vst3")

# Extract version components
string(REPLACE "." ";" VERSION_LIST ${PROJECT_VERSION})
list(GET VERSION_LIST 0 PROJECT_VERSION_MAJOR)
list(GET VERSION_LIST 1 PROJECT_VERSION_MINOR)
list(GET VERSION_LIST 2 PROJECT_VERSION_PATCH)

# Generate version header
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/tigerflame_build_info.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/tigerflame_build_info.h"
)

project(
    ${PROJECT_NAME}
    VERSION ${PROJECT_VERSION}
    DESCRIPTION ${PROJECT_DESCRIPTION}
    LANGUAGES CXX
)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Platform-specific settings
if(APPLE)
    set(CMAKE_MACOSX_DEPLOYMENT_TARGET 10.15)
    set(PLATFORMFLAGS "-fobjc-arc")
    add_compile_options(-Wall -Wextra -Wno-unused-parameter)
    add_compile_definitions(PLATFORM_MAC)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(COREAUDIO_LIBRARY CoreAudio)
    find_library(COREFOUNDATION_LIBRARY CoreFoundation)
    find_library(COREMIDI_LIBRARY CoreMIDI)
    find_library(METAL_LIBRARY Metal)
    find_library(QUARTZCORE_LIBRARY QuartzCore)
    set(PLATFORM_LIBS ${COCOA_LIBRARY} ${COREAUDIO_LIBRARY} ${COREFOUNDATION_LIBRARY} ${COREMIDI_LIBRARY} ${METAL_LIBRARY} ${QUARTZCORE_LIBRARY})
elseif(WIN32)
    add_compile_definitions(PLATFORM_WINDOWS NOMINMAX _CRT_SECURE_NO_WARNINGS)
    if(MSVC)
        set(PLATFORMFLAGS "/W4 /WX /wd4100 /wd4201 /wd4127 /wd4251 /wd4324 /wd4702 /wd4706")
        add_link_options(/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup)
        find_package(DirectX REQUIRED)
        set(PLATFORM_LIBS d3d11 dxgi)
    else()
        # MinGW / non-MSVC: DirectX SDK cmake config is not available in MinGW/MSYS2.
        # The editor UI backends (Phase 5) are stubs, so no platform graphics libs are
        # needed at link time for now. Update this block when the ImGui DX11 backend is
        # wired up under MSVC.
        set(PLATFORMFLAGS "-Wall -Wextra -Wno-unused-parameter")
        set(PLATFORM_LIBS "")
    endif()
elseif(UNIX AND NOT APPLE)
    set(PLATFORMFLAGS "-Wall -Wextra -Wno-unused-parameter")
    add_compile_definitions(PLATFORM_LINUX)
    find_package(OpenGL REQUIRED)
    find_package(X11 REQUIRED)
    set(PLATFORM_LIBS ${OPENGL_LIBRARIES} ${X11_LIBRARIES})
endif()

# Build options
option(BUILD_IMGUI_UI "Build with ImGui UI" ON)
option(BUILD_STANDALONE "Build standalone application" OFF)
option(BUILD_STANDALONE_MIDI "Build standalone with MIDI input" OFF)
option(BUILD_CLAP "Build CLAP plugin" OFF)
option(BUILD_TESTS "Build tests" ON)
option(BUILD_WITH_COVERAGE "Build with coverage flags" OFF)

# Debug/Release configurations
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g -O0)
    add_compile_definitions(DEBUG=1)
    if(BUILD_WITH_COVERAGE)
        add_compile_options(--coverage)
        add_link_options(--coverage)
    endif()
else()
    add_compile_options(-O2)
    if(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
        add_compile_options(-g)
    endif()
    add_compile_definitions(NDEBUG=1)
endif()

# Sanitizers
option(ENABLE_SANITIZER_ADDRESS "Enable AddressSanitizer" OFF)
option(ENABLE_SANITIZER_UNDEFINED "Enable UndefinedBehaviorSanitizer" OFF)
option(ENABLE_SANITIZER_LEAK "Enable LeakSanitizer" OFF)

if(ENABLE_SANITIZER_ADDRESS)
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
endif()

if(ENABLE_SANITIZER_UNDEFINED)
    add_compile_options(-fsanitize=undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=undefined)
endif()

if(ENABLE_SANITIZER_LEAK)
    add_compile_options(-fsanitize=leak -fno-omit-frame-pointer)
    add_link_options(-fsanitize=leak)
endif()

# FetchContent for dependencies
include(FetchContent)

# VST3 SDK
FetchContent_Declare(
    vst3sdk
    GIT_REPOSITORY https://github.com/steinbergmedia/vst3sdk.git
    GIT_TAG v3.7.7_build_19
    GIT_SHALLOW TRUE
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/vst3sdk
)

# Disable VSTGUI to avoid compilation issues with modern C++
set(SMTG_ADD_VSTGUI OFF CACHE BOOL "Disable VSTGUI")
# Disable hosting samples (includes validator which has compilation issues on some toolchains)
set(SMTG_ADD_VST3_HOSTING_SAMPLES OFF CACHE BOOL "Disable VST3 hosting samples")
set(SMTG_ENABLE_VST3_HOSTING_EXAMPLES OFF CACHE BOOL "Disable VST3 hosting examples")
# Disable utilities (includes moduleinfotool which fails to build on modern compilers)
set(SMTG_ADD_VST3_UTILITIES OFF CACHE BOOL "Disable VST3 utilities")
# Disable the VST3 validator post-build run: SMTG_RUN_VST_VALIDATOR causes the SDK to build
# and execute validator.exe after the plugin is compiled. Under MinGW on Windows the
# validator.exe crashes with 0xc0000139 (missing DLL entry point). Even on other platforms
# the validator requires the plugin to be fully installed, so disable it in CI.
set(SMTG_RUN_VST_VALIDATOR OFF CACHE BOOL "Disable running VST3 validator after build")

FetchContent_MakeAvailable(vst3sdk)

# Patch missing standard library includes in VST3 SDK files that fail to build on
# modern C++ standard libraries (GCC 13+, Clang 16+). These files are missing
# <cstdint> for uint32_t and <limits> for std::numeric_limits.
set(_vst3_moduleinfo_h "${vst3sdk_SOURCE_DIR}/public.sdk/source/vst/moduleinfo/moduleinfo.h")
if(EXISTS "${_vst3_moduleinfo_h}")
    file(READ "${_vst3_moduleinfo_h}" _content)
    if(NOT _content MATCHES "include <cstdint>")
        string(REPLACE "#include <vector>" "#include <vector>\n#include <cstdint>" _content "${_content}")
        file(WRITE "${_vst3_moduleinfo_h}" "${_content}")
        message(STATUS "Patched ${_vst3_moduleinfo_h} with missing <cstdint>")
    endif()
endif()
set(_vst3_parser_cpp "${vst3sdk_SOURCE_DIR}/public.sdk/source/vst/moduleinfo/moduleinfoparser.cpp")
if(EXISTS "${_vst3_parser_cpp}")
    file(READ "${_vst3_parser_cpp}" _content)
    if(NOT _content MATCHES "include <limits>")
        string(REPLACE "#include <stdexcept>" "#include <cstdint>\n#include <limits>\n#include <stdexcept>" _content "${_content}")
        file(WRITE "${_vst3_parser_cpp}" "${_content}")
        message(STATUS "Patched ${_vst3_parser_cpp} with missing <cstdint> and <limits>")
    endif()
endif()

include_directories(${vst3sdk_SOURCE_DIR})

# Dear ImGui
if(BUILD_IMGUI_UI)
    FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG v1.90.7
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(imgui)
    
    # Platform-specific ImGui backends
    if(APPLE)
        FetchContent_Declare(
            imgui-mac-metal
            GIT_REPOSITORY https://github.com/ocornut/imgui.git
            GIT_TAG v1.90.7
            GIT_SHALLOW TRUE
            SOURCE_SUBDIR examples/imgui_impl_metal.cpp
        )
        # We'll handle Metal backend separately
    elseif(WIN32)
        FetchContent_Declare(
            imgui-dx11
            GIT_REPOSITORY https://github.com/ocornut/imgui.git
            GIT_TAG v1.90.7
            GIT_SHALLOW TRUE
        )
    elseif(UNIX AND NOT APPLE)
        FetchContent_Declare(
            imgui-opengl3
            GIT_REPOSITORY https://github.com/ocornut/imgui.git
            GIT_TAG v1.90.7
            GIT_SHALLOW TRUE
        )
    endif()
endif()

# nlohmann/json for preset serialization
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(json)

# Catch2 for testing
if(BUILD_TESTS)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.5.0
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(Catch2)
endif()

# CLAP SDK
if(BUILD_CLAP)
    FetchContent_Declare(
        clap
        GIT_REPOSITORY https://github.com/free-audio/clap.git
        GIT_TAG main
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(clap)
    include_directories(${clap_SOURCE_DIR}/include)
endif()

# miniaudio for standalone audio
if(BUILD_STANDALONE)
    FetchContent_Declare(
        miniaudio
        GIT_REPOSITORY https://github.com/mackron/miniaudio.git
        GIT_TAG 0.11.14
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(miniaudio)
    include_directories(${miniaudio_SOURCE_DIR})
endif()

# RtMidi for standalone MIDI
if(BUILD_STANDALONE_MIDI)
    FetchContent_Declare(
        rtmidi
        GIT_REPOSITORY https://github.com/thestk/rtmidi.git
        GIT_TAG 5.0.0
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(rtmidi)
    include_directories(${rtmidi_SOURCE_DIR})
endif()

# Source files
set(CORE_SOURCES
    src/core/parameter_model.h
    src/core/parameter_model.cpp
    src/core/plugin_state.h
    src/core/plugin_state.cpp
    src/core/state.h
    src/core/state.cpp
    src/core/voice_allocator.h
    src/core/voice_allocator.cpp
    src/core/midi_translator.h
    src/core/midi_translator.cpp
    src/core/chip_engine.h
    src/core/chip_engine.cpp
    src/core/chip_bindings.h
    src/core/chip_bindings.cpp
    src/core/effect_chain.h
    src/core/effect_chain.cpp
    src/core/effects/qsound_effect.h
    src/core/effects/qsound_effect.cpp
    src/core/effects/reverb_effect.h
    src/core/effects/reverb_effect.cpp
    src/core/effects/delay_effect.h
    src/core/effects/delay_effect.cpp
    src/core/effects/distortion_effect.h
    src/core/effects/distortion_effect.cpp
    src/core/effects/eq_effect.h
    src/core/effects/eq_effect.cpp
    src/core/preset_manager.h
    src/core/preset_manager.cpp
)

set(PLUGIN_SOURCES
    src/plugin/ids.h
    src/plugin/version.h
    src/plugin/param_ids.h
    src/plugin/entry.cpp
    src/plugin/processor.h
    src/plugin/processor.cpp
    src/plugin/controller.h
    src/plugin/controller.cpp
)

set(UI_SOURCES
    src/ui/editor_core.h
    src/ui/editor_core.cpp
)

# Core library
add_library(tigerflame-core STATIC ${CORE_SOURCES})
set_target_properties(tigerflame-core PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(tigerflame-core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_BINARY_DIR}
        ${json_SOURCE_DIR}/include
)

target_compile_features(tigerflame-core PRIVATE cxx_std_17)

# mml2vgm-rs dependency (Phase 2 - not yet implemented)
# Placeholder: will be integrated via C FFI in future
set(MML2VGM_LIB "")
message(STATUS "mml2vgm-rs integration pending - using placeholder chip synthesis")

# VST3 plugin
add_library(tigerflame-vst3 MODULE ${PLUGIN_SOURCES} ${UI_SOURCES})

target_include_directories(tigerflame-vst3
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_BINARY_DIR}
        ${vst3sdk_SOURCE_DIR}
        ${vst3sdk_SOURCE_DIR}/public.sdk/source/vst
        ${vst3sdk_SOURCE_DIR}/public.sdk/source/main
        ${json_SOURCE_DIR}/include
)

target_link_libraries(tigerflame-vst3
    PRIVATE
        tigerflame-core
        sdk          # VST3 SDK: AudioEffect, EditController, ComponentBase, CPluginFactory
        ${PLATFORM_LIBS}
)

# Set VST3 plugin properties
set_target_properties(tigerflame-vst3 PROPERTIES
    PREFIX ""
    SUFFIX ".vst3"
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/VST3
    OUTPUT_NAME "TigerFlame"
)

# Platform-specific VST3 bundle structure
if(APPLE)
    set(VST3_BUNDLE_CONTENT_DIR ${CMAKE_CURRENT_BINARY_DIR}/VST3/TigerFlame.vst3/Contents)
    
    # Create Info.plist
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Info.plist.in"
        "${VST3_BUNDLE_CONTENT_DIR}/Info.plist"
    )
    
    # Create PkgInfo
    file(WRITE "${VST3_BUNDLE_CONTENT_DIR}/PkgInfo" "TYPE=Bundle\nBNDL????")
    
    # Structure: TigerFlame.vst3/Contents/MacOS/TigerFlame
    set_target_properties(tigerflame-vst3 PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${VST3_BUNDLE_CONTENT_DIR}/MacOS
        OUTPUT_NAME "TigerFlame"
    )
    
    # Ensure Resources directory exists at post-build time
    add_custom_command(TARGET tigerflame-vst3 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${VST3_BUNDLE_CONTENT_DIR}/Resources
        COMMENT "Creating VST3 bundle Resources directory for macOS"
    )
elseif(WIN32)
    set_target_properties(tigerflame-vst3 PROPERTIES
        OUTPUT_NAME "TigerFlame"
    )
    
    # Copy VST3 XML description
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/plugin.xml.in"
        "${CMAKE_CURRENT_BINARY_DIR}/VST3/TigerFlame/plugin.xml"
    )
endif()

# CLAP plugin
if(BUILD_CLAP)
    add_library(tigerflame-clap MODULE src/clap/clap_entry.cpp)
    
    target_include_directories(tigerflame-clap
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src
            ${CMAKE_CURRENT_BINARY_DIR}
            ${clap_SOURCE_DIR}/include
    )
    
    target_link_libraries(tigerflame-clap
        PRIVATE
            tigerflame-core
            ${PLATFORM_LIBS}
    )
    
    set_target_properties(tigerflame-clap PROPERTIES
        PREFIX ""
        SUFFIX ".clap"
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/CLAP
    )
endif()

# Standalone application
if(BUILD_STANDALONE)
    add_executable(tigerflame-standalone
        src/standalone/main.cpp
        src/standalone/audio_io.h
        src/standalone/audio_io.cpp
        src/standalone/midi_input.h
        src/standalone/midi_input.cpp
        src/standalone/runtime.h
        src/standalone/runtime.cpp
    )
    
    target_include_directories(tigerflame-standalone
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src
            ${CMAKE_CURRENT_BINARY_DIR}
            ${miniaudio_SOURCE_DIR}
    )
    
    target_link_libraries(tigerflame-standalone
        PRIVATE
            tigerflame-core
            ${PLATFORM_LIBS}
    )
    
    if(BUILD_STANDALONE_MIDI)
        target_include_directories(tigerflame-standalone
            PRIVATE
                ${rtmidi_SOURCE_DIR}
        )
        target_link_libraries(tigerflame-standalone
            PRIVATE
                rtmidi
        )
    endif()
endif()

# Tests
if(BUILD_TESTS)
    add_executable(tigerflame-tests
        tests/test_voice_allocator.cpp
        tests/test_midi_translator.cpp
        tests/test_plugin_state.cpp
        tests/test_chip_engine.cpp
        tests/test_audio_processing.cpp
        tests/test_effect_chain.cpp
        tests/main.cpp
    )
    
    target_include_directories(tigerflame-tests
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src
            ${CMAKE_CURRENT_BINARY_DIR}
            ${Catch2_SOURCE_DIR}/src
            ${Catch2_SOURCE_DIR}
            ${vst3sdk_SOURCE_DIR}
            ${json_SOURCE_DIR}/include
    )
    
    target_link_libraries(tigerflame-tests
        PRIVATE
            tigerflame-core
            Catch2::Catch2WithMain
    )

    # On MinGW, statically link the GCC and C++ runtimes into the test executable.
    # This avoids STATUS_ENTRYPOINT_NOT_FOUND (0xc0000139) at test runtime caused by
    # the Windows DLL loader finding an older libstdc++/libgcc (e.g. from Chocolatey's
    # MinGW) before MSYS2's versions in PATH.
    if(WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_link_options(tigerflame-tests PRIVATE
            -static-libgcc
            -static-libstdc++
        )
    endif()

    # Enable testing
    enable_testing()
    add_test(NAME tigerflame-tests COMMAND tigerflame-tests)
endif()

# Install targets
install(TARGETS tigerflame-vst3
    LIBRARY DESTINATION bin/VST3
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)

if(BUILD_CLAP)
    install(TARGETS tigerflame-clap
        LIBRARY DESTINATION bin/CLAP
    )
endif()

if(BUILD_STANDALONE)
    install(TARGETS tigerflame-standalone
        RUNTIME DESTINATION bin
    )
endif()
