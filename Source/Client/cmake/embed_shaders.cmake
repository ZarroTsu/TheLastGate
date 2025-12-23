function(generate_shader_embed_header SHADER_NAME)
    set(FRAG ${PROJECT_SOURCE_DIR}/resources/${SHADER_NAME}.frag)
    set(VERT ${PROJECT_SOURCE_DIR}/resources/${SHADER_NAME}.vert)
    set(OUT ${PROJECT_SOURCE_DIR}/_generated/shaders/${SHADER_NAME}_shader_files.h)
    set(TEMPLATE ../src/graphics/embedded_shader.h.in)

    message(STATUS "Checking shader ${SHADER_NAME}")
    message(STATUS "  FRAG = ${FRAG}")
    message(STATUS "  VERT = ${VERT}")

    if (NOT EXISTS ${FRAG})
        message(WARNING "FRAG DOES NOT EXIST: ${FRAG}")
    endif()

    if (NOT EXISTS ${VERT})
        message(WARNING "VERT DOES NOT EXIST: ${VERT}")
    endif()

    add_custom_command(
            OUTPUT ${OUT}
            COMMAND ${CMAKE_COMMAND}
            -DSHADER_NAME=${SHADER_NAME}
            -DFRAG=${FRAG}
            -DVERT=${VERT}
            -DTEMPLATE=${TEMPLATE}
            -DOUT=${OUT}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/generate_shader.cmake
            DEPENDS ${FRAG} ${VERT} ${TEMPLATE}
            COMMENT "Embedding shader ${SHADER_NAME}"
    )

    set(GENERATED_SHADERS ${GENERATED_SHADERS} ${OUT} PARENT_SCOPE)
endfunction()

generate_shader_embed_header(effect)
generate_shader_embed_header(effect_imgui)
generate_shader_embed_header(scaling)
generate_shader_embed_header(solid)
generate_shader_embed_header(magic)

add_custom_target(shader_embeds ALL DEPENDS ${GENERATED_SHADERS})

add_dependencies(TheLastGate shader_embeds)