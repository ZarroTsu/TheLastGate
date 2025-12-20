# tinyfiledialogs.cmake
#
# Build configuration for tinyfiledialogs library
# Cross-platform file dialog library

set(TINYFD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/tinyfiledialogs)

# tinyfiledialogs source files
set(TINYFD_SOURCES
    ${TINYFD_DIR}/tinyfiledialogs.c
)

# Create a static library for tinyfiledialogs
add_library(tinyfiledialogs STATIC
    ${TINYFD_SOURCES}
)

# Include directories for tinyfiledialogs
target_include_directories(tinyfiledialogs PUBLIC
    ${TINYFD_DIR}
)

# Disable warnings for tinyfiledialogs (it's third-party code)
if(MSVC)
    target_compile_options(tinyfiledialogs PRIVATE /W0)
else()
    target_compile_options(tinyfiledialogs PRIVATE -w)
endif()

# Link the tinyfiledialogs library to the main target
target_link_libraries(TheLastGate PRIVATE tinyfiledialogs)
