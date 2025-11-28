set(OUTPUT_DIR "$<TARGET_FILE_DIR:TheLastGate>")
add_custom_target(Package ALL
        COMMAND ${CMAKE_COMMAND} -E tar cfv TheLastGateSDLRelease.zip --format=zip

        "$<TARGET_FILE:TheLastGate>"
        "${OUTPUT_DIR}/gfx"
        "${OUTPUT_DIR}/sfx"
        "${OUTPUT_DIR}/resources"
        "${OUTPUT_DIR}/gx00.dat"
        "${OUTPUT_DIR}/gx00.idx"
        "${OUTPUT_DIR}/libpng16-16.dll"
        "${OUTPUT_DIR}/pnglib.dat"
        "${OUTPUT_DIR}/pnglib.idx"
        "${OUTPUT_DIR}/SDL2.dll"
        "${OUTPUT_DIR}/SDL2_image.dll"
        "${OUTPUT_DIR}/SDL2_mixer.dll"
        "${OUTPUT_DIR}/zlib1.dll"

        COMMENT "Creating release ZIP package..."
)

add_dependencies(Package TheLastGate)