#version 330 core

// ImGui's vertex format (different from the main effect shader!)
layout (location = 0) in vec2 aPos;        // ImGui uses vec2, not vec3
layout (location = 1) in vec2 aTexCoord;   // UV coords
layout (location = 2) in vec4 aColor;      // ImGui provides vertex color

out vec2 TexCoord;
out vec4 VertexColor;

uniform mat4 uProjection;
uniform mat4 uModel;

void main() {
    // Same as non-instanced path in effect.vert, but with vec2 position
    gl_Position = uProjection * uModel * vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
    VertexColor = aColor;
}
