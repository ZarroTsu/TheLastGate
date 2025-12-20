# SDL2 Setup - Cross-platform SDL2 detection and linking
# Uses pkg-config on all platforms (available in MSYS2/MinGW on Windows)

find_package(PkgConfig REQUIRED)

pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)
pkg_check_modules(SDL2_MIXER REQUIRED SDL2_mixer)
pkg_check_modules(SDL2_NET REQUIRED SDL2_net)

# Combine all SDL2 libraries
set(SDL2_LIBRARIES
    ${SDL2_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
    ${SDL2_MIXER_LIBRARIES}
    ${SDL2_NET_LIBRARIES}
)

# Add include directories
target_include_directories(TheLastGate PRIVATE
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_IMAGE_INCLUDE_DIRS}
    ${SDL2_MIXER_INCLUDE_DIRS}
    ${SDL2_NET_INCLUDE_DIRS}
)

# Add library directories
target_link_directories(TheLastGate PRIVATE
    ${SDL2_LIBRARY_DIRS}
    ${SDL2_IMAGE_LIBRARY_DIRS}
    ${SDL2_MIXER_LIBRARY_DIRS}
    ${SDL2_NET_LIBRARY_DIRS}
)

# Link SDL2 libraries
target_link_libraries(TheLastGate PUBLIC ${SDL2_LIBRARIES})

# Windows-specific: Add sdl2main for WinMain entry point
if(WIN32)
    target_link_libraries(TheLastGate PUBLIC sdl2main)
endif()
