static const char *effect_fragment =
    R"SHADER(#version 330 core

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
uniform bool uUseInstancing;

// Flag bit positions
#define FLAG_RED    (1 << 0)
#define FLAG_GREEN  (1 << 1)
#define FLAG_INVIS  (1 << 2)
#define FLAG_GREY   (1 << 3)
#define FLAG_INFRA  (1 << 4)
#define FLAG_WATER  (1 << 5)
#define FLAG_SHADOW (1 << 6)

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

    bool isRed, isGreen, isInvis, isGrey, isInfra, isWater, isShadow;
    if (useInstanceData) {
        isRed = (vFlags & FLAG_RED) != 0;
        isGreen = (vFlags & FLAG_GREEN) != 0;
        isInvis = (vFlags & FLAG_INVIS) != 0;
        isGrey = (vFlags & FLAG_GREY) != 0;
        isInfra = (vFlags & FLAG_INFRA) != 0;
        isWater = (vFlags & FLAG_WATER) != 0;
        isShadow = (vFlags & FLAG_SHADOW) != 0;
    } else {
        isRed = uRed;
        isGreen = uGreen;
        isInvis = uInvis;
        isGrey = uGrey;
        isInfra = uInfra;
        isWater = uWater;
        isShadow = uShadow;
    }

    if (isShadow) {
        color.rgb = vec3(0.0);
        color.a *= 0.5;
        FragColor = color;
        return;
    }

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
})SHADER";

static const char *effect_vertex =
    R"SHADER(#version 330 core

// Per-vertex attributes (from quad VBO)
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

// Per-instance attributes (from instance VBO)
layout (location = 2) in mat4 aInstanceModel;   // locations 2-5 (mat4 = 4 vec4s)
layout (location = 6) in vec2 aInstanceUV0;
layout (location = 7) in vec2 aInstanceUV1;
layout (location = 8) in float aInstanceGammaScale;
layout (location = 9) in float aInstanceShadeEffect;
layout (location = 10) in float aInstanceGammaEffect;
layout (location = 11) in int aInstanceFlags;

out vec2 TexCoord;
out float vGammaScale;
out float vShadeEffect;
out float vGammaEffect;
flat out int vFlags;

uniform mat4 uProjection;
uniform mat4 uModel;  // For non-instanced rendering
uniform bool uUseInstancing;

void main() {
    if (uUseInstancing) {
        // Instanced rendering path
        gl_Position = uProjection * aInstanceModel * vec4(aPos, 1.0);
        TexCoord = mix(aInstanceUV0, aInstanceUV1, aTexCoord);
        vGammaScale = aInstanceGammaScale;
        vShadeEffect = aInstanceShadeEffect;
        vGammaEffect = aInstanceGammaEffect;
        vFlags = aInstanceFlags;
    } else {
        // Legacy uniform rendering path
        gl_Position = uProjection * uModel * vec4(aPos, 1.0);
        TexCoord = aTexCoord;
        vGammaScale = 0.0;  // Signal to fragment shader to use uniforms
        vShadeEffect = 0.0;
        vGammaEffect = 0.0;
        vFlags = 0;
    }
})SHADER";
