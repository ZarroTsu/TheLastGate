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
