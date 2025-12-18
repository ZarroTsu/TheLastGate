cmake_minimum_required(VERSION 3.22)
project(TheLastGate LANGUAGES C CXX)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/compiler_options.cmake)

add_executable(TheLastGate WIN32
        engine.c
        engine.h
        src/graphics/atlas.c
        src/graphics/atlas.h
        src/graphics/loader.c
        src/graphics/loader.h
        src/graphics/scaling.c
        src/graphics/sdl.c
        src/graphics/sdl.h
        src/graphics/shaders/effect_shader.c
        src/graphics/shaders/effect_shader.h
        src/graphics/shaders/magic_shader.c
        src/graphics/shaders/shaders.c
        src/graphics/shaders/shaders.h
        src/graphics/shaders/solid_shader.c
        src/graphics/shaders/solid_shader.h
        src/graphics/sprite_data.h
        input.c
        input.h
        inter.c
        main.c
        main.h
        main.h
        MERC.rc
        src/mods/base_cost_warning.c
        src/mods/base_cost_warning.h
        src/mods/give_more.c
        src/mods/give_more.h
        src/mods/use_queue.c
        src/mods/use_queue.h
        options.c
        src/graphics/render.c
        src/graphics/render.h
        src/graphics/sdl_magic.c
        src/net/connection.c
        src/net/socket.c
        src/audio/sound.c
        src/audio/sound.h
        src/ui/option_window.c
        src/ui/option_window.h
        src/ui/widgets.c
        src/config/keybindings.c
        src/config/keybindings.h
        src/ui/spell_hud.c
        src/config/config.c
        src/log/log.c
        src/security/security.c
        src/security/single_instance.c
        src/security/unique_id.c
        src/mods/stubborn_actions.c
        src/ui/ui.c
)

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
            -DHOMECOPY
    )
endif ()

# Release build optimizations
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(TheLastGate PRIVATE
            -O3                    # Maximum optimization
            -march=x86-64          # Optimize for current CPU (or use -march=x86-64 for compatibility)
            -ffast-math            # Fast floating-point math (safe for game logic)
            -flto                  # Link-time optimization
            -DNDEBUG               # Disable assertions (enables the printf macro suppression)
    )
    target_link_options(TheLastGate PRIVATE
            -flto                  # Link-time optimization
            -s                     # Strip debug symbols
    )
endif ()

# Profile-Guided Optimization (PGO) builds
# Stage 1: Build with profiling instrumentation
if (CMAKE_BUILD_TYPE STREQUAL "PGO-Generate")
    target_compile_options(TheLastGate PRIVATE
            -O3
            -march=x86-64
            -ffast-math
            -fprofile-generate     # Generate profiling data
            -DNDEBUG
    )
    target_link_options(TheLastGate PRIVATE
            -fprofile-generate
    )
endif ()

# Stage 2: Build using collected profile data
if (CMAKE_BUILD_TYPE STREQUAL "PGO-Use")
    target_compile_options(TheLastGate PRIVATE
            -O3
            -march=x86-64
            -ffast-math
            -flto
            -fprofile-use          # Use profiling data for optimization
            -fprofile-correction   # Handle profile inconsistencies gracefully
            -DNDEBUG
    )
    target_link_options(TheLastGate PRIVATE
            -flto
            -fprofile-use
            -s
    )
endif ()

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/link_libraries.cmake)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/imgui.cmake)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/copy_runtime_dlls.cmake)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/include_packaged_files.cmake)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/package_target.cmake)