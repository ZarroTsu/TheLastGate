# Windows-only: Package Windows release with DLLs
if (WIN32)
    set(OUTPUT_DIR "$<TARGET_FILE_DIR:TheLastGate>")
    add_custom_target(Package ALL
            COMMAND ${CMAKE_COMMAND} -E tar cfv TheLastGateSDLRelease.zip --format=zip

            "$<TARGET_FILE:TheLastGate>"
            "${OUTPUT_DIR}/gfx"
            "${OUTPUT_DIR}/ui"
            "${OUTPUT_DIR}/sfx"
            "${OUTPUT_DIR}/resources"
            "${OUTPUT_DIR}/gx00.dat"
            "${OUTPUT_DIR}/gx00.idx"
            "${OUTPUT_DIR}/pnglib.dat"
            "${OUTPUT_DIR}/pnglib.idx"

            # SDL2 libraries
            "${OUTPUT_DIR}/SDL2.dll"
            "${OUTPUT_DIR}/SDL2_image.dll"
            "${OUTPUT_DIR}/SDL2_mixer.dll"
            "${OUTPUT_DIR}/SDL2_net.dll"

            # Image support (PNG only)
            "${OUTPUT_DIR}/libpng16.dll"
            "${OUTPUT_DIR}/libzlib1.dll"

            # Audio codecs (Vorbis is hard dependency of SDL2_mixer in vcpkg)
            "${OUTPUT_DIR}/libogg.dll"
            "${OUTPUT_DIR}/libvorbis.dll"
            "${OUTPUT_DIR}/libvorbisenc.dll"
            "${OUTPUT_DIR}/libvorbisfile.dll"

            # MinGW runtime libraries
            "${OUTPUT_DIR}/libgcc_s_seh-1.dll"
            "${OUTPUT_DIR}/libstdc++-6.dll"
            "${OUTPUT_DIR}/libwinpthread-1.dll"

            # Imgui runtime dependencies
            "${OUTPUT_DIR}/libfreetype.dll"
            "${OUTPUT_DIR}/libbz2.dll"
            "${OUTPUT_DIR}/libbrotlicommon.dll"
            "${OUTPUT_DIR}/libbrotlidec.dll"
            "${OUTPUT_DIR}/libbrotlienc.dll"

            COMMENT "Creating release ZIP package..."
    )

    add_dependencies(Package TheLastGate)
else ()
    # Linux: Create AppImage (single portable file with everything)
    # Note: Requires linuxdeploy to be installed
    # The AppImage target is defined in cmake/appimage.cmake

    add_custom_target(Package
            COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target AppImage
            COMMENT "Creating AppImage package (run 'cmake --build build --target AppImage' manually if needed)"
    )

    add_dependencies(Package TheLastGate)

    # Also provide a simple tar.gz as fallback
    set(OUTPUT_DIR "$<TARGET_FILE_DIR:TheLastGate>")
    add_custom_target(TarPackage
            COMMAND ${CMAKE_COMMAND} -E echo "Creating tar.gz package..."
            COMMAND ${CMAKE_COMMAND} -E tar czf TheLastGateSDLRelease.tar.gz --format=gnutar
                "$<TARGET_FILE:TheLastGate>"
                "${OUTPUT_DIR}/gfx"
                "${OUTPUT_DIR}/sfx"
                "${OUTPUT_DIR}/resources"
                "${OUTPUT_DIR}/gx00.dat"
                "${OUTPUT_DIR}/gx00.idx"
                "${OUTPUT_DIR}/pnglib.dat"
                "${OUTPUT_DIR}/pnglib.idx"
            COMMENT "Creating tar.gz package with all data files"
    )
    add_dependencies(TarPackage TheLastGate)
endif ()