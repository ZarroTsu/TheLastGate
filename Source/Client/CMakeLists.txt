cmake_minimum_required(VERSION 3.22)
project(TheLastGate C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE C_SOURCES "*.c")
file(GLOB_RECURSE C_HEADERS "*.h")
set_source_files_properties(${C_SOURCES} PROPERTIES LANGUAGE C)
set_source_files_properties(${C_HEADERS} PROPERTIES LANGUAGE C)

add_executable(TheLastGate WIN32
        engine.c
        engine.h
        graphics/atlas.c
        graphics/atlas.h
        graphics/loader.c
        graphics/loader.h
        graphics/sdl.c
        graphics/sdl.h
        graphics/shaders.c
        graphics/shaders.h
        graphics/sprite_data.h
        input.c
        input.h
        inter.c
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
        log.h
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

# Add include directories for libpng and zlib
target_include_directories(TheLastGate PRIVATE
        ../../Resources/lpng
        ${CMAKE_CURRENT_SOURCE_DIR}/zlib
        ${CMAKE_CURRENT_SOURCE_DIR}/_generated
)

# OpenGL
find_package(OpenGL REQUIRED)

# GLAD
add_library(glad external/glad/src/glad.c)
target_include_directories(glad PUBLIC external/glad/include)

#target_link_libraries(TheLastGate PRIVATE Dbghelp)

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

target_link_libraries(TheLastGate PRIVATE
        glad
        OpenGL::GL
)

if (WIN32)
    target_link_libraries(TheLastGate PRIVATE user32)
endif ()

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

function(generate_shader_embed_header SHADER_NAME)
    set(FRAG ${PROJECT_SOURCE_DIR}/resources/${SHADER_NAME}.frag)
    set(VERT ${PROJECT_SOURCE_DIR}/resources/${SHADER_NAME}.vert)
    set(OUT ${PROJECT_SOURCE_DIR}/_generated/shaders/${SHADER_NAME}_shader_files.h)
    set(TEMPLATE ${PROJECT_SOURCE_DIR}/graphics/embedded_shader.h.in)

    add_custom_command(
            OUTPUT ${OUT}
            COMMAND ${CMAKE_COMMAND}
                -DSHADER_NAME=${SHADER_NAME}
                -DFRAG=${FRAG}
                -DVERT=${VERT}
                -DTEMPLATE=${TEMPLATE}
                -DOUT=${OUT}
                -P ${PROJECT_SOURCE_DIR}/cmake/generate_shader.cmake
            DEPENDS ${FRAG} ${VERT} ${TEMPLATE}
            COMMENT "Embedding shader ${SHADER_NAME}"
    )

    set(GENERATED_SHADERS ${GENERATED_SHADERS} ${OUT} PARENT_SCOPE)
endfunction()

generate_shader_embed_header(effect)
generate_shader_embed_header(solid)

add_custom_target(shader_embeds ALL DEPENDS ${GENERATED_SHADERS})

set(OUTPUT_DIR "$<TARGET_FILE_DIR:TheLastGate>")
add_custom_target(Package ALL
        COMMAND ${CMAKE_COMMAND} -E tar cfv TheLastGateSDLRelease.zip --format=zip

        "$<TARGET_FILE:TheLastGate>"
        "${OUTPUT_DIR}/gfx"
        "${OUTPUT_DIR}/sfx"
        "${OUTPUT_DIR}/resources"
        "${OUTPUT_DIR}/gx00.dat"
        "${OUTPUT_DIR}/gx00.idx"
        "${OUTPUT_DIR}/libpng16-16.dll"
        "${OUTPUT_DIR}/pnglib.dat"
        "${OUTPUT_DIR}/pnglib.idx"
        "${OUTPUT_DIR}/SDL2.dll"
        "${OUTPUT_DIR}/SDL2_image.dll"
        "${OUTPUT_DIR}/SDL2_mixer.dll"
        "${OUTPUT_DIR}/zlib1.dll"

        COMMENT "Creating release ZIP package..."
)

add_dependencies(Package TheLastGate)