cmake_minimum_required(VERSION 3.22)

# vcpkg integration - must be set before project()
# Set CMAKE_TOOLCHAIN_FILE if not already set (allows command-line override)
if (NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if (DEFINED ENV{VCPKG_ROOT})
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
                CACHE STRING "vcpkg toolchain file")
    else ()
        message(WARNING "VCPKG_ROOT environment variable not set. "
                "Please set it or specify CMAKE_TOOLCHAIN_FILE manually. "
                "Example: cmake -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake ..")
    endif ()
endif ()

# Set default triplet based on platform and compiler
# MinGW needs x64-mingw-dynamic, MSVC needs x64-windows, Linux needs x64-linux
if (NOT DEFINED VCPKG_TARGET_TRIPLET)
    if (MINGW)
        set(VCPKG_TARGET_TRIPLET "x64-mingw-dynamic" CACHE STRING "vcpkg target triplet")
        message(STATUS "Detected MinGW compiler, using vcpkg triplet: x64-mingw-dynamic")
    elseif (UNIX AND NOT APPLE)
        set(VCPKG_TARGET_TRIPLET "x64-linux" CACHE STRING "vcpkg target triplet")
        message(STATUS "Detected Linux, using vcpkg triplet: x64-linux")
    else ()
        set(VCPKG_TARGET_TRIPLET "x64-windows" CACHE STRING "vcpkg target triplet")
        message(STATUS "Using vcpkg triplet: x64-windows")
    endif ()
endif ()

project(TheLastGate LANGUAGES C CXX)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/compiler_options.cmake)

# Platform-specific source files
if (WIN32)
    set(PLATFORM_SOURCES TheLastGate.rc)
else ()
    set(PLATFORM_SOURCES)
endif ()

add_executable(TheLastGate
        ${PLATFORM_SOURCES}
        engine.c
        inter.c
        main.c
        options.c
        src/audio/sound.c
        src/config/config.c
        src/config/keybindings.c
        src/game/game_input.c
        src/game/game_ui.c
        src/game/stat_display.c
        src/graphics/atlas.c
        src/graphics/item_display.c
        src/graphics/loader.c
        src/graphics/render.c
        src/graphics/scaling.c
        src/graphics/sdl.c
        src/graphics/sdl_magic.c
        src/graphics/shaders/effect_shader.c
        src/graphics/shaders/magic_shader.c
        src/graphics/shaders/shaders.c
        src/graphics/shaders/solid_shader.c
        src/input/input.c
        src/launcher/character_manager.c
        src/launcher/launcher.c
        src/launcher/launcher_input.c
        src/log/log.c
        src/mods/base_cost_warning.c
        src/mods/give_more.c
        src/mods/stubborn_actions.c
        src/mods/use_queue.c
        src/net/connection.c
        src/net/socket.c
        src/security/security.c
        src/security/single_instance.c
        src/security/unique_id.c
        src/ui/hotbar.c
        src/ui/inventory.c
        src/ui/option_window.c
        src/ui/perf_window.c
        src/ui/ui.c
        src/ui/widgets.c
        src/util/perf.c
)

# Use C++ linker for mixed C/C++ project (C++ linker handles C code correctly)
# This is REQUIRED because we link against imgui_wrapper (C++ library)
# The C++ linker can link both C and C++ object files without issues
set_target_properties(TheLastGate PROPERTIES LINKER_LANGUAGE CXX)

# Disable warnings for old code compatibility
# TODO: These warnings indicate real issues that should be fixed during modernization
target_compile_options(TheLastGate PRIVATE
        -Wno-incompatible-pointer-types  # Disable pointer type mismatch warnings
        -Wno-int-conversion              # Disable int/pointer conversion warnings (often related)
)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/opengl.cmake)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/embed_shaders.cmake)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/windows_setup.cmake)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(TheLastGate PRIVATE
#            -DHOMECOPY
    )
endif ()

# Release build optimizations
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(TheLastGate PRIVATE
            -O3                    # Maximum optimization
            -march=x86-64          # Optimize for x86-64 compatibility
            -ffast-math            # Fast floating-point math (safe for game logic)
            -DNDEBUG               # Disable assertions (enables the printf macro suppression)
    )

    target_link_options(TheLastGate PRIVATE
            -s                     # Strip debug symbols
    )
endif ()

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/sdl2_setup.cmake)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/link_libraries.cmake)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/imgui.cmake)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/tinyfiledialogs.cmake)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/copy_runtime_dlls.cmake)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/include_packaged_files.cmake)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/appimage.cmake)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/package_target.cmake)