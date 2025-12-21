# SDL2 Setup - vcpkg-based SDL2 detection and linking
# Uses modern CMake imported targets from vcpkg packages

# SDL2 core
find_package(SDL2 CONFIG REQUIRED)

# SDL2 extensions
find_package(SDL2_image CONFIG REQUIRED)
find_package(SDL2_mixer CONFIG REQUIRED)
find_package(SDL2_net CONFIG REQUIRED)

# Link SDL2 libraries using modern imported targets
# vcpkg provides these targets with automatic debug/release selection
# Different platforms may use different target naming conventions

# Determine correct target names based on what's available
if(TARGET SDL2_image::SDL2_image)
    set(SDL2_IMAGE_LIB SDL2_image::SDL2_image)
else()
    set(SDL2_IMAGE_LIB SDL2_image)
endif()

if(TARGET SDL2_mixer::SDL2_mixer)
    set(SDL2_MIXER_LIB SDL2_mixer::SDL2_mixer)
else()
    set(SDL2_MIXER_LIB SDL2_mixer)
endif()

if(TARGET SDL2_net::SDL2_net)
    set(SDL2_NET_LIB SDL2_net::SDL2_net)
else()
    set(SDL2_NET_LIB SDL2_net)
endif()

target_link_libraries(TheLastGate PRIVATE
    SDL2::SDL2main
    SDL2::SDL2
    ${SDL2_IMAGE_LIB}
    ${SDL2_MIXER_LIB}
    ${SDL2_NET_LIB}
)

message(STATUS "SDL2 libraries linked via vcpkg")
