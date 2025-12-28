vcpkg_check_linkage(ONLY_STATIC_LIBRARY)

# This overlay uses pre-generated GLAD files with OpenGL 4.6 Core profile
# The files should be placed in glad-core/ subdirectory
# Generate them at: https://glad.dav1d.de/
# Settings: Language=C/C++, Specification=OpenGL, gl=Version 4.6, Profile=Core

set(SOURCE_PATH "${CMAKE_CURRENT_LIST_DIR}/glad-core")

# Check if pre-generated files exist
if(NOT EXISTS "${SOURCE_PATH}/include/glad/glad.h")
    message(FATAL_ERROR "GLAD core profile files not found. Please generate them:
    1. Visit https://glad.dav1d.de/
    2. Select: Language=C/C++, Specification=OpenGL
    3. API: gl Version 4.6, Profile=Core
    4. Generator: C/C++
    5. Add extensions as needed
    6. Download and extract to: ${CMAKE_CURRENT_LIST_DIR}/glad-core/
    ")
endif()

# Create CMakeLists.txt for GLAD
file(WRITE "${SOURCE_PATH}/CMakeLists.txt" [[
cmake_minimum_required(VERSION 3.10)
project(glad C)

add_library(glad STATIC src/glad.c)

target_include_directories(glad PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Installation
install(TARGETS glad EXPORT gladConfig
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

install(DIRECTORY include/ DESTINATION include)

install(EXPORT gladConfig
    FILE gladConfig.cmake
    NAMESPACE glad::
    DESTINATION share/glad
)
]])

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_build()

vcpkg_cmake_install()

vcpkg_cmake_config_fixup(PACKAGE_NAME glad CONFIG_PATH share/glad)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

# Copy license from glad-core directory if it exists, otherwise create a simple one
if(EXISTS "${SOURCE_PATH}/LICENSE")
    file(INSTALL "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)
else()
    file(WRITE "${CURRENT_PACKAGES_DIR}/share/${PORT}/copyright" "GLAD is public domain / MIT licensed\nSee https://github.com/Dav1dde/glad\n")
endif()

vcpkg_copy_pdbs()
