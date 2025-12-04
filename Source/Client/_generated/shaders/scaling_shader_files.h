static const char *scaling_fragment =
    R"SHADER(#version 330 core

in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uTexture;

void main() {
    FragColor = texture(uTexture, vUV);
})SHADER";

static const char *scaling_vertex =
    R"SHADER(#version 330 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;

out vec2 vUV;

void main() {
    vUV = aUV;
    gl_Position = vec4(aPos, 0.0, 1.0);
})SHADER";
