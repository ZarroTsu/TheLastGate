# SDL2 Setup - vcpkg-based SDL2 detection and linking
# Uses modern CMake imported targets from vcpkg packages

# SDL2 core
find_package(SDL2 CONFIG REQUIRED)

# SDL2 extensions
find_package(SDL2_image CONFIG REQUIRED)
find_package(SDL2_mixer CONFIG REQUIRED)
find_package(SDL2_net CONFIG REQUIRED)

# Link SDL2 libraries using modern imported targets
# vcpkg provides these targets:
#   SDL2::SDL2main - SDL2 main entry point wrapper (WinMain on Windows)
#   SDL2::SDL2      - SDL2 core library
target_link_libraries(TheLastGate PRIVATE
    SDL2::SDL2main  # Provides WinMain wrapper for Windows
    SDL2::SDL2
    SDL2_image
    SDL2_mixer
    SDL2_net
)

message(STATUS "SDL2 libraries linked via vcpkg")
