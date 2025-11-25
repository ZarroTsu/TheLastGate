#include "shaders.h"

#include <stdio.h>
#include <stdlib.h>

GLuint load_shader(const char *path, GLenum type) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        printf("ERROR: Shader file not found: %s\n", path);
        return 0;
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    char *src = malloc(size + 1);
    fread(src, 1, size, f);
    src[size] = '\0';
    fclose(f);

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, (const GLchar* const*)&src, NULL);
    glCompileShader(shader);

    // Check compilation status
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        printf("ERROR: Shader compilation failed (%s):\n%s\n", path, infoLog);
        free(src);
        glDeleteShader(shader);
        return 0;
    }

    free(src);
    return shader;
}

GLuint load_program(const char *vpath, const char *fpath) {
    GLuint vs = load_shader(vpath, GL_VERTEX_SHADER);
    GLuint fs = load_shader(fpath, GL_FRAGMENT_SHADER);

    if (!vs || !fs) {
        printf("ERROR: Failed to load shaders for program\n");
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
        printf("ERROR: Shader program linking failed:\n%s\n", infoLog);
        glDeleteProgram(program);
        glDeleteShader(vs);
        glDeleteShader(fs);
        return 0;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    printf("Shader program loaded successfully\n");
    return program;
}
