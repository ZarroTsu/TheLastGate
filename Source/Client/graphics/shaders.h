#ifndef THELASTGATE_SHADERS_H
#define THELASTGATE_SHADERS_H
#include "glad/glad.h"

typedef enum TLG_ShaderTypes {
    TLG_Shader_Effect,
    TLG_Shader_Solid
} TLG_ShaderTypes;

GLuint load_program(TLG_ShaderTypes shader_type);


#endif //THELASTGATE_SHADERS_H
