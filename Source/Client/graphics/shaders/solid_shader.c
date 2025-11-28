#include "solid_shader.h"

#include <stdio.h>

#include "shaders.h"
#include "glad/glad.h"
#include "shaders/solid_shader_files.h"

static GLuint shader;

static GLint uProjection = -1;
static GLint uModel = -1;
static GLint uColor = -1;

void load_solid_shader(float projection_matrix[16]) {
    shader = load_shader_program(solid_vertex, solid_fragment);
    if (!shader) {
        printf("Unable to load solid shader\n");
        return;
    }

    glUseProgram(shader);

    uProjection = glGetUniformLocation(shader, "uProjection");
    uModel = glGetUniformLocation(shader, "uModel");
    uColor = glGetUniformLocation(shader, "uColor");

    glUniformMatrix4fv(uProjection, 1, GL_FALSE, projection_matrix);

    glUseProgram(0);
}

void drop_solid_shader(void) {
    if (shader) {
        glDeleteProgram(shader);
        shader = 0;
    }
}

GLint use_solid_shader(RGBAColor color) {
    glUseProgram(shader);

    // Always Reset Values
    glUniform4f(uColor, 0.0f, 0.0f, 0.0f, 0.0f);

    glUniform4f(uColor, color.r, color.g, color.b, color.a);
    return uModel;
}
