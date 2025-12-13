# Add include directories for libpng and zlib
target_include_directories(TheLastGate PRIVATE
        ../../Resources/lpng
        ${CMAKE_CURRENT_SOURCE_DIR}/_generated
        ${CMAKE_CURRENT_SOURCE_DIR}/src/
        ${CMAKE_CURRENT_SOURCE_DIR}/
)

target_link_libraries(TheLastGate PUBLIC
        # Core libraries
        sdl2        # SDL2 core (will link statically if available)
        sdl2_image  # SDL2_image (dynamic - too many format dependencies)
        sdl2_mixer  # SDL2_mixer (dynamic - audio format dependencies)
        sdl2_net    # SDL2_net (cross-platform networking)
        png         # libpng
        z           # zlib


        # Legacy DirectDraw/DirectSound
        ddraw
        dsound

        # Windows system libraries (for static SDL2 core)
        ws2_32
)