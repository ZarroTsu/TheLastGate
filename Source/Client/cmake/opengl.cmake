# OpenGL
find_package(OpenGL REQUIRED)

# GLAD
add_library(glad ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/src/glad.c)

target_include_directories(glad PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include)

target_link_libraries(TheLastGate PRIVATE
        glad
        OpenGL::GL
)
