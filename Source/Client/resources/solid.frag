#version 330 core

out vec4 FragColor;

uniform vec4 uColor;  // RGBA color (r, g, b, a)

void main() {
    FragColor = uColor;
}
