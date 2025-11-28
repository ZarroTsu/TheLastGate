# Recursively copy the packaged files directory
add_custom_command(TARGET TheLastGate POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/../../Resources/Packaged Files"
        $<TARGET_FILE_DIR:TheLastGate>
        COMMENT "Recursively copying Packaged Files to output directory"
)