#pragma once
#include <stdbool.h>

#include "glad/glad.h"

typedef struct {
    float gamma_scale;
    float uv0[2];
    float uv1[2];
    float shade_effect;
    float gamma_effect;
    bool red;
    bool green;
    bool invis;
    bool grey;
    bool infra;
    bool water;
    bool shadow;
} EffectShaderSettings;

void load_effect_shader(float projection_matrix[16]);
void drop_effect_shader(void);

GLint use_effect_shader(EffectShaderSettings settings);
void use_effect_shader_instanced(void);
