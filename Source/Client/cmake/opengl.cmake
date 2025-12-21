# OpenGL and GLAD setup - vcpkg-based OpenGL loader

# OpenGL - system-provided, use standard CMake find module
find_package(OpenGL REQUIRED)

# GLAD - use vcpkg package instead of vendored version
find_package(glad CONFIG REQUIRED)

# Link OpenGL and GLAD using imported targets
target_link_libraries(TheLastGate PRIVATE
    OpenGL::GL
    glad::glad
)

message(STATUS "OpenGL and GLAD linked via vcpkg")
