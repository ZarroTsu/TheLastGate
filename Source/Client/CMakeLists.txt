cmake_minimum_required(VERSION 3.22)
project(TheLastGate C)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/compiler_options.cmake)

add_executable(TheLastGate WIN32
        engine.c
        engine.h
        graphics/atlas.c
        graphics/atlas.h
        graphics/loader.c
        graphics/loader.h
        graphics/scaling.c
        graphics/sdl.c
        graphics/sdl.h
        graphics/shaders/effect_shader.c
        graphics/shaders/effect_shader.h
        graphics/shaders/magic_shader.c
        graphics/shaders/shaders.c
        graphics/shaders/shaders.h
        graphics/shaders/solid_shader.c
        graphics/shaders/solid_shader.h
        graphics/sprite_data.h
        input.c
        input.h
        inter.c
        log.h
        main.c
        main.h
        main.h
        MERC.rc
        options.c
        render.c
        render.h
        sdl_magic.c
        socket.c
        socket.h
        sound.c
        sound.h
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

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/copy_runtime_dlls.cmake)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/include_packaged_files.cmake)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/package_target.cmake)