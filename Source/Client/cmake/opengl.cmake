# OpenGL and GLAD setup - vcpkg-based OpenGL loader

# OpenGL - system-provided, use standard CMake find module
find_package(OpenGL REQUIRED)

# GLAD - use vcpkg package instead of vendored version
find_package(glad CONFIG REQUIRED)


# Determine correct target names based on what's available
if(TARGET OpenGL::GL)
    set(OPENGL_LIB OpenGL::GL)
else()
    set(OPENGL_LIB GL)
endif()

if(TARGET glad::glad)
    set(GLAD_LIB glad::glad)
else()
    set(GLAD_LIB glad)
endif()
# Link OpenGL and GLAD using imported targets
target_link_libraries(TheLastGate PRIVATE
    ${OPENGL_LIB}
    ${GLAD_LIB}
)

message(STATUS "OpenGL and GLAD linked via vcpkg")
