#ifndef THELASTGATE_SHADERS_H
#define THELASTGATE_SHADERS_H
#include "glad/glad.h"

GLuint load_shader(const char *path, GLenum type);
GLuint load_program(const char *vpath, const char *fpath);

#endif //THELASTGATE_SHADERS_H
