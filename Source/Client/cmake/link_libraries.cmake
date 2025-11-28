# Add include directories for libpng and zlib
target_include_directories(TheLastGate PRIVATE
        ../../Resources/lpng
        ${CMAKE_CURRENT_SOURCE_DIR}/zlib
        ${CMAKE_CURRENT_SOURCE_DIR}/_generated
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