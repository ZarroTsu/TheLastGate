cmake_minimum_required(VERSION 3.22)
project(TheLastGate C)

add_executable(TheLastGate WIN32
        conv.c
        dd.c
        engine.c
        inter.c
        main.c
        options.c
        socket.c
        sound.c
        MERC.rc
        render.h
        render.c
        sdl.c
        sdl.h
        sdl_magic.c
        input.c
        input.h
        engine.h
        main.h
        socket.h
        main.h
        sound.h
)

# Set Windows subsystem to GUI (uses WinMain instead of main)
set_target_properties(TheLastGate PROPERTIES
        WIN32_EXECUTABLE TRUE
)

# Add linker flags for Windows GUI subsystem and hybrid static/dynamic linking
target_link_options(TheLastGate PRIVATE
        -mwindows           # MinGW: Use WinMain entry point, no console window
)

# Disable warnings for old code compatibility
# TODO: These warnings indicate real issues that should be fixed during modernization
target_compile_options(TheLastGate PRIVATE
        -Wno-incompatible-pointer-types  # Disable pointer type mismatch warnings
        -Wno-int-conversion              # Disable int/pointer conversion warnings (often related)
)

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

# Add include directories for libpng and zlib
target_include_directories(TheLastGate PRIVATE
        ../../Resources/lpng
        ${CMAKE_CURRENT_SOURCE_DIR}/zlib
)

target_link_libraries(TheLastGate PUBLIC
        # Core libraries
        sdl2        # SDL2 core (will link statically if available)
        sdl2_image  # SDL2_image (dynamic - too many format dependencies)
        sdl2_mixer  # SDL2_mixer (dynamic - audio format dependencies)
        png         # libpng
        z           # zlib


        # Legacy DirectDraw/DirectSound
        ddraw
        dsound

        # Windows system libraries (for static SDL2 core)
        ws2_32
)

# Hybrid linking: Copy only SDL2_image, SDL2_mixer, and their format library DLLs
if (MINGW)
    get_filename_component(MINGW_BIN_DIR ${CMAKE_C_COMPILER} DIRECTORY)

    # Minimal DLL set (SDL2_image/mixer and their direct dependencies)
    set(REQUIRED_DLLS
            #        SDL2.dll
            #        SDL2_image.dll
            #        SDL2_mixer.dll
            # Common
            libpng16-16.dll
            zlib1.dll
    )

    # Copy DLLs to lib/ subdirectory
    foreach (DLL ${REQUIRED_DLLS})
        add_custom_command(TARGET TheLastGate POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:TheLastGate>
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${MINGW_BIN_DIR}/${DLL}"
                $<TARGET_FILE_DIR:TheLastGate>/${DLL}
                COMMAND echo "Copied ${DLL}"
                COMMENT "Copying ${DLL}"
        )
    endforeach ()
endif ()

# Recursively copy the packaged files directory
add_custom_command(TARGET TheLastGate POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/../../Resources/Packaged Files"
        $<TARGET_FILE_DIR:TheLastGate>
        COMMENT "Recursively copying Packaged Files to output directory"
)

add_custom_command(TARGET TheLastGate POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/../../Resources/DLLs/SDL"
        $<TARGET_FILE_DIR:TheLastGate>
        COMMENT "Recursively copying DLLs to output directory"
)