static const char *magic_fragment =
    R"SHADER(#version 330 core

in vec2 vUV;
out vec4 FragColor;

uniform vec3 uColor;
uniform float uStrength;
uniform float uRadius;

vec2 computePixelPosition(vec2 uv, float widthScale, float heightScale) {
    vec2 pos = uv * 64.0;

    // center at (32,32), scale outward/inward
    pos.x = (pos.x - 32.0) * widthScale + 32.0;
    pos.y = (pos.y - 32.0) * heightScale + 32.0;

    return pos;
}

// ------------------------------------------------------------
// Compute each directional falloff (0 → 1)
// LEFT / RIGHT fade over 32px
// TOP fade over ~21px
// BOTTOM fade over ~23px (×2 intensity slope)
// ------------------------------------------------------------
float computeLeftFall(float x) { return max(0.0, (32.0 - x) / 32); }
float computeRightFall(float x) { return max(0.0, (x - 31.0) / 32); }
float computeTopFall(float y) { return max(0.0, (16.0 - y) / 21); }
float computeBottomFall(float y) { return max(0.0, (y - 55.0) / 23.0) * 2.0; }

float computeNormalizedMask(float leftFall, float rightFall, float topFall, float bottomFall) {
    float mask = 1.0 - leftFall - rightFall - topFall - bottomFall;

    return clamp(mask, 0.0, 1.0);
}

float computeGlow(float normalizedMask, float strength) {
    return clamp(normalizedMask / strength, 0.0, 1.0);
}

vec3 applyGlow(vec3 baseColor, vec3 glowColor, float glow) {
    vec3 result = baseColor - (glowColor * glow);
    result = max(result, 0.0);

    result += glowColor * glow;
    return clamp(result, 0.0, 1.0);
}

void main() {

    vec2 pos = computePixelPosition(vUV, 1.54, 1.09);
    float x = pos.x;
    float y = pos.y;

    float leftFall = computeLeftFall(x);
    float rightFall = computeRightFall(x);
    float topFall = computeTopFall(y);
    float bottomFall = computeBottomFall(y);

    float normalizedMask = computeNormalizedMask(leftFall, rightFall, topFall, bottomFall);

    float glow = computeGlow(normalizedMask, uStrength);

    vec3 finalColor = applyGlow(vec3(0.0), uColor, glow);
    FragColor = vec4(finalColor, 1);

})SHADER";

static const char *magic_vertex =
    R"SHADER(#version 330 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aUV;

out vec2 vUV;

uniform mat4 uProjection;
uniform mat4 uModel;

void main() {
    vUV = aUV;
    gl_Position = uProjection * uModel * vec4(aPos, 0.0, 1.0);
})SHADER";
