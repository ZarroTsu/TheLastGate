#pragma once
#include "shaders.h"
#include "glad/glad.h"

void load_solid_shader(float projection_matrix[16]);
void drop_solid_shader(void);

GLint use_solid_shader(RGBAColor color);