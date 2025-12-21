# Copy runtime DLLs - vcpkg-based DLL deployment
# Automatically copies all required DLLs to the build output directory

if(WIN32)
    # vcpkg provides a helper function to copy all DLLs
    # This is set up by the vcpkg toolchain file

    # For x64-windows triplet (dynamic linking), vcpkg automatically:
    # 1. Installs DLLs to vcpkg_installed/x64-windows/bin/
    # 2. Provides CMAKE_PREFIX_PATH for find_package
    # 3. Handles DLL deployment via fixup_bundle or manual copy

    # Copy all required DLLs from vcpkg to output directory
    # This works for SDL2, SDL2_image, SDL2_mixer, SDL2_net, libpng, zlib, etc.

    # Get all linked libraries' DLL paths
    # vcpkg sets the IMPORTED_LOCATION property for DLLs

    # Helper: Copy DLLs for a target's dependencies
    # Use debug DLLs for Debug builds, release DLLs for Release builds
    add_custom_command(TARGET TheLastGate POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Copying vcpkg DLLs to output directory..."

        # vcpkg installs debug DLLs to vcpkg_installed/<triplet>/debug/bin/
        # and release DLLs to vcpkg_installed/<triplet>/bin/
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "$<IF:$<CONFIG:Debug>,${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/debug/bin,${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/bin>"
            "$<TARGET_FILE_DIR:TheLastGate>"

        COMMENT "Copying runtime DLLs from vcpkg ($<CONFIG> build)"
    )

    # Also copy MinGW runtime DLLs if using MinGW compiler
    if(MINGW)
        get_filename_component(MINGW_BIN_DIR ${CMAKE_C_COMPILER} DIRECTORY)

        set(MINGW_RUNTIME_DLLS
            libgcc_s_seh-1.dll
            libstdc++-6.dll
            libwinpthread-1.dll
        )

        foreach(DLL ${MINGW_RUNTIME_DLLS})
            add_custom_command(TARGET TheLastGate POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${MINGW_BIN_DIR}/${DLL}"
                    "$<TARGET_FILE_DIR:TheLastGate>/${DLL}"
                COMMENT "Copying MinGW runtime: ${DLL}"
            )
        endforeach()
    endif()

    message(STATUS "Configured DLL deployment from vcpkg")
endif()
