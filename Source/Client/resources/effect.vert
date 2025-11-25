#version 330 core

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
}