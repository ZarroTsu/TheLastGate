vcpkg_check_linkage(ONLY_STATIC_LIBRARY)

# Fetch ImGui from master branch
vcpkg_from_github(
        OUT_SOURCE_PATH SOURCE_PATH
        REPO ocornut/imgui
        REF v1.92.5
        HEAD_REF master
        SHA512 382b862a285464bd311c79a0ff07885e42300d79704bb65cd1cbbf35cef63f7f50784ed23f7479e4490bbaae0d23ea1b2b067a3571e0b442d390824f9611bd59
)

# Create CMakeLists.txt for building ImGui as a library
file(WRITE "${SOURCE_PATH}/CMakeLists.txt" [[
cmake_minimum_required(VERSION 3.10)
project(imgui CXX)

# Core ImGui files
set(IMGUI_SOURCES
    imgui.cpp
    imgui_demo.cpp
    imgui_draw.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
)

set(IMGUI_HEADERS
    imgui.h
    imconfig.h
    imgui_internal.h
    imstb_rectpack.h
    imstb_textedit.h
    imstb_truetype.h
)

# Backend files
set(BACKEND_SOURCES
    backends/imgui_impl_sdl2.cpp
    backends/imgui_impl_opengl3.cpp
)

set(BACKEND_HEADERS
    backends/imgui_impl_sdl2.h
    backends/imgui_impl_opengl3.h
)

find_package(SDL2 CONFIG REQUIRED)

add_library(imgui STATIC ${IMGUI_SOURCES} ${BACKEND_SOURCES})
target_include_directories(imgui PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/backends>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(imgui PUBLIC SDL2::SDL2)

# Installation
install(TARGETS imgui EXPORT imguiConfig
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES ${IMGUI_HEADERS} DESTINATION include)
install(FILES ${BACKEND_HEADERS} DESTINATION include)

install(EXPORT imguiConfig
    FILE imguiConfig.cmake
    NAMESPACE imgui::
    DESTINATION share/imgui
)
]])

vcpkg_cmake_configure(
        SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_build()

vcpkg_cmake_install()

vcpkg_cmake_config_fixup(PACKAGE_NAME imgui CONFIG_PATH share/imgui)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

# Handle copyright
file(INSTALL "${SOURCE_PATH}/LICENSE.txt" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)

vcpkg_copy_pdbs()
