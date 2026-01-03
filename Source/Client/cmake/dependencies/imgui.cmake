# imgui.cmake
#
# ImGui configuration using vcpkg package
# This replaces the vendored ImGui build with vcpkg's pre-built package

# Find FreeType (required by ImGui for font rasterization)
find_package(Freetype REQUIRED)

# Find ImGui package with SDL2 and OpenGL3 backends
find_package(imgui CONFIG REQUIRED)

# vcpkg's imgui package provides these targets:
#   imgui::imgui - core ImGui library (imgui.cpp, imgui_draw.cpp, etc.)
#
# Backends are included in the core library when features are enabled:
#   - sdl2-binding: includes imgui_impl_sdl2.cpp
#   - opengl3-binding: includes imgui_impl_opengl3.cpp

# Link ImGui to main target
target_link_libraries(TheLastGate PRIVATE imgui::imgui)

# We still need our C wrapper (imgui_wrapper.cpp)
# Build it separately and link it
set(IMGUI_WRAPPER_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/imgui/imgui_wrapper.cpp)

# Create a small library for just the C wrapper
add_library(imgui_wrapper_c STATIC ${IMGUI_WRAPPER_SOURCE})

# Set C++ standard for wrapper compilation
set_target_properties(imgui_wrapper_c PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS YES
)

# The wrapper needs to include imgui headers (provided by imgui::imgui target)
target_link_libraries(imgui_wrapper_c PRIVATE imgui::imgui)

# The wrapper also needs OpenGL definitions (for IMGUI_IMPL_OPENGL_LOADER_GLAD)
# and FreeType support
target_compile_definitions(imgui_wrapper_c PUBLIC
    IMGUI_IMPL_OPENGL_LOADER_GLAD
    IMGUI_ENABLE_FREETYPE
)

# Disable warnings for wrapper (it interfaces with third-party code)
if(MSVC)
    target_compile_options(imgui_wrapper_c PRIVATE /W0)
else()
    target_compile_options(imgui_wrapper_c PRIVATE -w)
endif()

# Link the C wrapper to the main target
target_link_libraries(TheLastGate PRIVATE imgui_wrapper_c)

message(STATUS "ImGui linked via vcpkg (with custom C wrapper)")
