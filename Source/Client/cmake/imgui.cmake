# imgui.cmake
#
# Build configuration for Dear ImGui and its C wrapper
# This is the ONLY place where C++ compilation happens in the project

set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui)

# Dear ImGui core source files
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# C++ wrapper source
set(IMGUI_WRAPPER_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/imgui/imgui_wrapper.cpp
)

# Create a static library for ImGui
# This library is compiled as C++ but exposes a C interface
add_library(imgui_wrapper STATIC
    ${IMGUI_SOURCES}
    ${IMGUI_WRAPPER_SOURCES}
)

# Set C++ standard for ImGui compilation
set_target_properties(imgui_wrapper PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

# Include directories for ImGui
target_include_directories(imgui_wrapper PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui  # For imgui_wrapper.h
    ${IMGUI_DIR}                          # For imgui.h
    ${IMGUI_DIR}/backends                 # For backends
)

# Find SDL2 (should already be found by main project, but ensure it's available)
find_package(SDL2 REQUIRED)
target_include_directories(imgui_wrapper PRIVATE ${SDL2_INCLUDE_DIRS})

# Link OpenGL (GLAD is already included in main project)
# ImGui needs to know about OpenGL but doesn't link directly
target_compile_definitions(imgui_wrapper PUBLIC
    IMGUI_IMPL_OPENGL_LOADER_GLAD
)

# Disable warnings for ImGui (it's third-party code)
if(MSVC)
    target_compile_options(imgui_wrapper PRIVATE /W0)
else()
    target_compile_options(imgui_wrapper PRIVATE -w)
endif()

# Link the ImGui wrapper library to the main target
target_link_libraries(TheLastGate PRIVATE imgui_wrapper)
