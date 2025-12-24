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

target_link_libraries(TheLastGate PRIVATE
    SDL2::SDL2main
    SDL2::SDL2
    $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>
    $<IF:$<TARGET_EXISTS:SDL2_mixer::SDL2_mixer>,SDL2_mixer::SDL2_mixer,SDL2_mixer::SDL2_mixer-static>
    $<IF:$<TARGET_EXISTS:SDL2_net::SDL2_net>,SDL2_net::SDL2_net,SDL2_net::SDL2_net-static>
)

message(STATUS "SDL2 libraries linked via vcpkg")
