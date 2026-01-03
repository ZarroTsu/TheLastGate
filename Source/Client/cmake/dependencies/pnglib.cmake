# Find libpng and zlib via vcpkg (fixes broken ../../Resources/lpng path)
find_package(PNG REQUIRED)
find_package(ZLIB REQUIRED)

# Link libpng and zlib using imported targets
# PNG::PNG automatically includes zlib as a transitive dependency
target_link_libraries(TheLastGate PRIVATE
        PNG::PNG
        ZLIB::ZLIB
)

message(STATUS "libpng and zlib linked via vcpkg")