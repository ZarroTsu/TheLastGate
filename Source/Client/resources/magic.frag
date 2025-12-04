#version 330 core

in vec2 vUV;
out vec4 FragColor;

uniform vec3 uColor;
uniform float uStrength;
uniform float uRadius;

void main() {
    vec2 p = vUV * 2.0 - 1.0;

    float dx = (vUV.x * 64.0) - 32.0;
    float dy = (vUV.y * 64.0) - 32.0;

    float nx = dx / 32.0;
    float ny = dy / 64.0;

    float r = length(vec2(nx, ny)) * 1.5;

    float t = clamp(1.0 - r, 0.0, 1.0);
    float glow = pow(t, 2);
    glow *= 1.5;
    float fade = 1.0 - (max(uStrength, 1) / 7.0);

    FragColor = vec4(uColor * glow * fade, glow * fade * 3);
}