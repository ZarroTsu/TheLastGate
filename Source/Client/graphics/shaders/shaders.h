#pragma once
#include "glad/glad.h"

typedef struct {
    float r, g, b;
} RGBColor;

typedef struct {
    float r, g, b, a;
} RGBAColor;

GLuint load_shader_program(const char *vertex_source, const char *fragment_source);
