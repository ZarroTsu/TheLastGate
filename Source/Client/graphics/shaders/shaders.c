#include "shaders.h"

#include <stdio.h>
#include <stdlib.h>
#include "../../log.h"
#include "../../main.h"
#include "shaders/effect_shader_files.h"
#include "shaders/solid_shader_files.h"
#include "shaders/magic_shader_files.h"

static GLuint magic_shader;

static GLuint load_shader(const char *source, GLenum type) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, (const GLchar* const*)&source, NULL);
    glCompileShader(shader);

    // Check compilation status
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        LOG("ERROR: Shader compilation failed (%s):\n%s\n", app_state.path, infoLog);
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

GLuint load_shader_program(const char *vertex_source, const char *fragment_source) {
    GLuint vs = load_shader(vertex_source, GL_VERTEX_SHADER);
    GLuint fs = load_shader(fragment_source, GL_FRAGMENT_SHADER);

    if (!vs || !fs) {
        LOG("ERROR: Failed to load shaders for program\n");
        if (vs) glDeleteShader(vs);
        if (fs) glDeleteShader(fs);
        return 0;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    // Check linking status
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        LOG("ERROR: Shader program linking failed:\n%s\n", infoLog);
        glDeleteProgram(program);
        glDeleteShader(vs);
        glDeleteShader(fs);
        return 0;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    LOG("Shader program loaded successfully\n");
    return program;
}

GLuint load_program(TLG_ShaderTypes shader_type) {
    switch (shader_type) {
        case TLG_Shader_Effect:
            return load_shader_program(effect_vertex, effect_fragment);
        case TLG_Shader_Solid:
            return load_shader_program(solid_vertex, solid_fragment);
        default:
            LOG("Unknown shader type\n");
            return 0;
    }
}