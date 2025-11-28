#include "magic_shader.h"

#include <stdio.h>

#include "../sdl.h"
#include "shaders.h"
#include "glad/glad.h"
#include "shaders/magic_shader_files.h"

static GLuint shader;

static GLint uProjection = -1;
static GLint uModel = -1;
static GLint uColor = -1;
static GLint uStrength = -1;
static GLint uRadius = -1;

void load_magic_shader(float project_matrix[16]) {
    shader = load_shader_program(magic_vertex, magic_fragment);
    if (!shader) {
        printf("Unable to load magic shader\n");
        return;
    }

    glUseProgram(shader);

    uProjection = glGetUniformLocation(shader, "uProjection");
    uModel = glGetUniformLocation(shader, "uModel");
    uColor = glGetUniformLocation(shader, "uColor");
    uStrength = glGetUniformLocation(shader, "uStrength");
    uRadius = glGetUniformLocation(shader, "uRadius");

    glUniformMatrix4fv(uProjection, 1, GL_FALSE, project_matrix);

    glUseProgram(0);
}

void drop_magic_shader(void) {
    if (shader) {
        glDeleteProgram(shader);
        shader = 0;
    }
}

GLint use_magic_shader(const RGBColor color, const int strength) {
    glUseProgram(shader);

    // Reset all parameters always
    glUniform3f(uColor, 0.0f, 0.0f, 0.0f);
    glUniform1f(uStrength, 0.0f);
    glUniform1f(uRadius, 0.0f);

    glUniform3f(uColor, color.r, color.g, color.b);
    glUniform1f(uStrength, (float)strength);

    return uModel;
}
