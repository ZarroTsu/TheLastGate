#pragma once
#include "shaders.h"

typedef struct {
    float color[3];
    float strength;
    float radius;
} MagicShaderSettings;

void load_magic_shader(float projection_matrix[16]);

void drop_magic_shader(void);

// Using a shader configures the shader and returns the model uniform location
GLint use_magic_shader(RGBColor color, int strength);
