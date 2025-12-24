# Link libraries - vcpkg-based dependency linking

# Find libpng and zlib via vcpkg (fixes broken ../../Resources/lpng path)
find_package(PNG REQUIRED)
find_package(ZLIB REQUIRED)

# Link libpng and zlib using imported targets
# PNG::PNG automatically includes zlib as a transitive dependency
target_link_libraries(TheLastGate PRIVATE
    PNG::PNG
)

# Add include directories for project structure
target_include_directories(TheLastGate PRIVATE
    ${CMAKE_BINARY_DIR}/_generated
    ${CMAKE_CURRENT_SOURCE_DIR}/src/
    ${CMAKE_CURRENT_SOURCE_DIR}/
)

# Platform-specific libraries
if(WIN32)
    target_link_libraries(TheLastGate PRIVATE
        ws2_32      # Windows socket library
    )
endif()

message(STATUS "libpng and zlib linked via vcpkg")
