# Add include directories for libpng and zlib
target_include_directories(TheLastGate PRIVATE
        ../../Resources/lpng
        ${CMAKE_CURRENT_SOURCE_DIR}/_generated
        ${CMAKE_CURRENT_SOURCE_DIR}/src/
        ${CMAKE_CURRENT_SOURCE_DIR}/
)

# SDL2 libraries are handled by cmake/sdl2_setup.cmake

target_link_libraries(TheLastGate PUBLIC
        # Core cross-platform libraries
        png         # libpng
        z           # zlib
)

# Platform-specific libraries
if(WIN32)
    target_link_libraries(TheLastGate PUBLIC
            ws2_32      # Windows socket library
    )
endif()