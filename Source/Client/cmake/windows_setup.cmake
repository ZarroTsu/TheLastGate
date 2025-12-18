if (WIN32)
    # Set Windows subsystem to GUI (uses WinMain instead of main)
    set_target_properties(TheLastGate PROPERTIES
            WIN32_EXECUTABLE TRUE
    )

    # Add linker flags for Windows GUI subsystem and hybrid static/dynamic linking
    target_link_options(TheLastGate PRIVATE
            -mwindows           # MinGW: Use WinMain entry point, no console window
    )

    target_link_libraries(TheLastGate PRIVATE
            user32
            bcrypt
    )
endif ()