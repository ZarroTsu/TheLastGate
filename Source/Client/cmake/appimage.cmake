# AppImage packaging for Linux
# Creates a single portable executable containing the game and all dependencies

if (NOT WIN32)
    # AppImage is only for Linux
    # Find linuxdeploy tool
    find_program(LINUXDEPLOY linuxdeploy HINTS $ENV{HOME}/bin /usr/local/bin)

    if (NOT LINUXDEPLOY)
        message(WARNING "linuxdeploy not found in PATH. AppImage target will be available but may fail.")
        message(WARNING "Install from: https://github.com/linuxdeploy/linuxdeploy/releases")
    else ()
        message(STATUS "Found linuxdeploy: ${LINUXDEPLOY}")
    endif ()

    set(APPIMAGE_NAME "TheLastGate-x86_64.AppImage")
    set(APPDIR "${CMAKE_BINARY_DIR}/AppDir")

    # Create .desktop file for AppImage
    file(WRITE "${CMAKE_BINARY_DIR}/TheLastGate.desktop"
            "[Desktop Entry]
Type=Application
Name=The Last Gate
Comment=MMORPG Game Client
Exec=TheLastGate
Icon=TheLastGate
Categories=Game;
Terminal=false
")

    # Create AppRun script (entry point for AppImage)
    file(WRITE "${CMAKE_BINARY_DIR}/AppRun"
            "#!/bin/bash
SELF=\$(readlink -f \"\$0\")
HERE=\${SELF%/*}
export PATH=\"\${HERE}/usr/bin:\${PATH}\"
export LD_LIBRARY_PATH=\"\${HERE}/usr/lib:\${LD_LIBRARY_PATH}\"
# Run from usr/bin where executable and data files are located
cd \"\${HERE}/usr/bin\"
exec \"\${HERE}/usr/bin/TheLastGate\" \"\$@\"
")

    # Custom target to build AppImage
    add_custom_target(AppImage
            COMMAND ${CMAKE_COMMAND} -E echo "Building AppImage..."

            # Create AppDir structure
            COMMAND ${CMAKE_COMMAND} -E make_directory "${APPDIR}/usr/bin"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${APPDIR}/usr/lib"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${APPDIR}/usr/share"

            # Copy executable
            COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:TheLastGate>" "${APPDIR}/usr/bin/"

            # Copy data files to usr/bin/ (same directory as executable)
            # The game uses SDL_GetBasePath() which returns the executable's directory
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "$<TARGET_FILE_DIR:TheLastGate>/gfx" "${APPDIR}/usr/bin/gfx"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "$<TARGET_FILE_DIR:TheLastGate>/sfx" "${APPDIR}/usr/bin/sfx"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "$<TARGET_FILE_DIR:TheLastGate>/resources" "${APPDIR}/usr/bin/resources"

            # Copy .dat/.idx files to usr/bin/
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE_DIR:TheLastGate>/gx00.dat" "${APPDIR}/usr/bin/"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE_DIR:TheLastGate>/gx00.idx" "${APPDIR}/usr/bin/"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE_DIR:TheLastGate>/pnglib.dat" "${APPDIR}/usr/bin/"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE_DIR:TheLastGate>/pnglib.idx" "${APPDIR}/usr/bin/"

            # Copy .desktop file, icon, and AppRun to AppDir root
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/TheLastGate.desktop" "${APPDIR}/"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/TheLastGate.png" "${APPDIR}/"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/AppRun" "${APPDIR}/"
            COMMAND chmod +x "${APPDIR}/AppRun"

            # Use linuxdeploy to gather dependencies and create AppImage
            COMMAND ${CMAKE_COMMAND} -E echo "Running linuxdeploy to gather dependencies..."
            COMMAND ${LINUXDEPLOY} --appdir=${APPDIR} --output appimage

            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Creating AppImage package..."
    )

    add_dependencies(AppImage TheLastGate)

    # Print instructions
    add_custom_command(TARGET AppImage POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo ""
            COMMAND ${CMAKE_COMMAND} -E echo "========================================="
            COMMAND ${CMAKE_COMMAND} -E echo "AppImage created: ${APPIMAGE_NAME}"
            COMMAND ${CMAKE_COMMAND} -E echo "========================================="
            COMMAND ${CMAKE_COMMAND} -E echo "To test: ./${APPIMAGE_NAME}"
            COMMAND ${CMAKE_COMMAND} -E echo ""
    )

endif ()
