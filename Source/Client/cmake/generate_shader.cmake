file(READ ${FRAG} FRAGMENT_SHADER_CONTENT)
file(READ ${VERT} VERTEX_SHADER_CONTENT)

set(FRAGMENT_SHADER_NAME "${SHADER_NAME}_fragment")
set(VERTEX_SHADER_NAME "${SHADER_NAME}_vertex")

configure_file(
        ${TEMPLATE}
        ${OUT}
        @ONLY
)