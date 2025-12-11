# Hybrid linking: Copy only SDL2_image, SDL2_mixer, and their format library DLLs
if (MINGW)
    get_filename_component(MINGW_BIN_DIR ${CMAKE_C_COMPILER} DIRECTORY)

    set(REQUIRED_DLLS
            # Common
            libpng16-16.dll
            zlib1.dll
            libgcc_s_seh-1.dll
            libstdc++-6.dll
            libwinpthread-1.dll
    )

    # Copy DLLs to lib/ subdirectory
    foreach (DLL ${REQUIRED_DLLS})
        add_custom_command(TARGET TheLastGate POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:TheLastGate>
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${MINGW_BIN_DIR}/${DLL}"
                $<TARGET_FILE_DIR:TheLastGate>/${DLL}
                COMMAND echo "Copied ${DLL}"
                COMMENT "Copying ${DLL}"
        )
    endforeach ()
endif ()

# We use the MVSC SDL dependencies whe we ship so we don't need to ship a million other dlls with them.
add_custom_command(TARGET TheLastGate POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/../../Resources/DLLs/SDL"
        $<TARGET_FILE_DIR:TheLastGate>
        COMMENT "Recursively copying DLLs to output directory"
)