#version 330 core

// Vertex attributes
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;  // Unused, but present in quad VBO

// Uniforms
uniform mat4 uProjection;
uniform mat4 uModel;

void main() {
    gl_Position = uProjection * uModel * vec4(aPos, 1.0);
}
