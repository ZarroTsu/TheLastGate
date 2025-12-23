#version 330 core

in vec2 TexCoord;
in vec4 VertexColor;

out vec4 FragColor;

uniform sampler2D uTexture;
// All the same uniforms as the non-instanced path in effect.frag
uniform vec2 uUV0;
uniform vec2 uUV1;
uniform float uGammaScale;
uniform float uGammaEffect;
uniform float uShadeEffect;
uniform bool uRed;
uniform bool uGreen;
uniform bool uInvis;
uniform bool uGrey;
uniform bool uInfra;
uniform bool uWater;
uniform bool uShadow;
uniform bool uBuff;

void main() {
    // Use the same UV remapping logic as non-instanced path
    vec2 uv = mix(uUV0, uUV1, TexCoord);

    vec4 color = texture(uTexture, uv);

    // Extract effect parameters (non-instanced path)
    float gammaScale = uGammaScale;
    float shadeEffect = uShadeEffect;
    float gammaEffect = uGammaEffect;

    bool isRed = uRed;
    bool isGreen = uGreen;
    bool isInvis = uInvis;
    bool isGrey = uGrey;
    bool isInfra = uInfra;
    bool isWater = uWater;
    bool isShadow = uShadow;
    bool isBuff = uBuff;

    // All the same effect logic from effect.frag
    if (isShadow) {
        color.rgb = vec3(0.0);
        color.a *= 0.5;
        FragColor = color * VertexColor;
        return;
    }

    if (isBuff) gammaEffect = 120.0;

    if (gammaEffect > 0.0 && shadeEffect > 0.0) {
        color.rgb *= gammaEffect / (shadeEffect * shadeEffect + gammaEffect);
    }
    if (isGrey) {
        float tmp = (color.r + color.g + color.b) / 3.0 * 0.5;
        color.r = tmp;
        color.g = tmp;
        color.b = tmp;
    }
    if (isInfra) {
        float tmp = (color.r + color.g + color.b) / 3.0;
        color.r = tmp;
        color.g = 0.0;
        color.b = 0.0;
    }
    if (isWater) {
        float tmp = (color.r + color.g + color.b) / 3.0;
        color.r = (color.r + tmp) / 3.0;
        color.g = (color.g + tmp) / 2.0;
        color.b = color.b + tmp * 1.2;

        color.r = clamp(color.r, 0.0, 1.0);
        color.g = clamp(color.g, 0.0, 1.0);
        color.b = clamp(color.b, 0.0, 1.0);
    }
    color.rgb *= gammaScale;
    if (isRed) {
        color.rgb *= 2.0;
    }

    if (isGreen) {
        color.g += 0.5;
    }

    if (isInvis) {
        color.a = 0.0;
    }

    // Multiply by ImGui's vertex color (for proper blending)
    FragColor = color * VertexColor;
}
