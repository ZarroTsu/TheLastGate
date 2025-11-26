#version 330 core

in vec2 TexCoord;
in float vGammaScale;
in float vShadeEffect;
in float vGammaEffect;
flat in int vFlags;

out vec4 FragColor;

uniform sampler2D uTexture;
// Keep old uniforms for backward compatibility (non-batched rendering)
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
uniform bool uUseInstancing;

// Flag bit positions
#define FLAG_RED    (1 << 0)
#define FLAG_GREEN  (1 << 1)
#define FLAG_INVIS  (1 << 2)
#define FLAG_GREY   (1 << 3)
#define FLAG_INFRA  (1 << 4)
#define FLAG_WATER  (1 << 5)
#define FLAG_SHADOW (1 << 6)
#define FLAG_BUFF   (1 << 7)

void main() {
    // Use instance data if available (vGammaScale will be > 0 for instanced rendering)
    // Otherwise fall back to uniforms for backward compatibility
    bool useInstanceData = uUseInstancing;

    vec2 uv;
    if (useInstanceData) {
        uv = TexCoord; // Already interpolated in vertex shader
    } else {
        uv = mix(uUV0, uUV1, TexCoord); // Legacy path
    }

    vec4 color = texture(uTexture, uv);

    // Extract effect parameters
    float gammaScale = useInstanceData ? vGammaScale : uGammaScale;
    float shadeEffect = useInstanceData ? vShadeEffect : uShadeEffect;
    float gammaEffect = useInstanceData ? vGammaEffect : uGammaEffect;

    bool isRed, isGreen, isInvis, isGrey, isInfra, isWater, isShadow, isBuff;
    if (useInstanceData) {
        isRed = (vFlags & FLAG_RED) != 0;
        isGreen = (vFlags & FLAG_GREEN) != 0;
        isInvis = (vFlags & FLAG_INVIS) != 0;
        isGrey = (vFlags & FLAG_GREY) != 0;
        isInfra = (vFlags & FLAG_INFRA) != 0;
        isWater = (vFlags & FLAG_WATER) != 0;
        isShadow = (vFlags & FLAG_SHADOW) != 0;
        isBuff = (vFlags & FLAG_BUFF) != 0;
    } else {
        isRed = uRed;
        isGreen = uGreen;
        isInvis = uInvis;
        isGrey = uGrey;
        isInfra = uInfra;
        isWater = uWater;
        isShadow = uShadow;
        isBuff = uBuff;
    }

    if (isShadow) {
        color.rgb = vec3(0.0);
        color.a *= 0.5;
        FragColor = color;
        return;
    }

    if (isBuff) gammaEffect = 1;

    if (gammaEffect > 0.0 && shadeEffect > 0.0) {
        color.rgb *= gammaEffect / (shadeEffect * shadeEffect + gammaEffect);
    }
    if (isGrey) {
        float tmp = (color.r + (color.g / 2.0) + color.b) / 6.0;
        color.r = tmp;
        color.g = tmp * 2.0;
        color.b = tmp;
    }
    if (isInfra) {
        float tmp = (color.r + (color.g / 2.0) + color.b) / 3.0;
        color.r = tmp;
        color.g = 0.0;
        color.b = 0.0;
    }
    if (isWater) {
        float tmp = (color.r + (color.g / 2.0) + color.b) / 2.0;
        color.r = (color.r + tmp) / 3.0;
        color.g = ((color.g + tmp) / 3.0) * 2.0;
        color.b = color.b + tmp;
    }
    color.rgb *= gammaScale;
    if (isRed) {
        color.rgb *= 2.0;
    }

    if (isGreen) {
        color.g += 0.5; // 128/255 ≈ 0.5
    }

    if (isInvis) {
        color.a = 0.0;
    }
    FragColor = color;
}