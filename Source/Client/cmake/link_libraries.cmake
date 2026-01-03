# Link libraries - vcpkg-based dependency linking

# Add include directories for project structure
target_include_directories(TheLastGate PRIVATE
        ${CMAKE_BINARY_DIR}/_generated
        ${CMAKE_CURRENT_SOURCE_DIR}/src/
        ${CMAKE_CURRENT_SOURCE_DIR}/
)