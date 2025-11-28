#pragma once
#include "glad/glad.h"

typedef enum TLG_ShaderTypes {
    TLG_Shader_Effect,
    TLG_Shader_Solid,
    TLG_Shader_Magic
} TLG_ShaderTypes;

typedef struct {
    float r, g, b;
} RGBColor;

typedef struct {
    float r, g, b, a;
} RGBAColor;

GLuint load_program(TLG_ShaderTypes shader_type);

GLuint load_shader_program(const char *vertex_source, const char *fragment_source);
