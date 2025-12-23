//
// Created by james on 11/28/2025.
//

#include "effect_shader.h"

#include <stdio.h>

#include "shaders.h"
#include "shaders/effect_shader_files.h"
#include "shaders/effect_imgui_shader_files.h"

static GLuint shader;

static GLint uProjection = -1;
static GLint uModel = -1;
static GLint uTexture = -1;
static GLint uUV0 = -1;
static GLint uUV1 = -1;
static GLint uGammaScale = -1;
static GLint uGammaEffect = -1;
static GLint uShadeEffect = -1;
static GLint uRed = -1;
static GLint uGreen = -1;
static GLint uInvis = -1;
static GLint uGrey = -1;
static GLint uInfra = -1;
static GLint uWater = -1;
static GLint uShadow = -1;
static GLint uUseInstancing = -1;

void load_effect_shader(float projection_matrix[16]) {
    shader = load_shader_program(effect_vertex, effect_fragment);
    if (!shader) {
        printf("Failed to load gamma shader\n");
        return;
    }

    glUseProgram(shader);

    // Cache uniform locations (query once, not every frame!)
    uProjection = glGetUniformLocation(shader, "uProjection");
    uUV0 = glGetUniformLocation(shader, "uUV0");
    uUV1 = glGetUniformLocation(shader, "uUV1");
    uModel = glGetUniformLocation(shader, "uModel");
    uTexture = glGetUniformLocation(shader, "uTexture");
    uGammaScale = glGetUniformLocation(shader, "uGammaScale");
    uGammaEffect = glGetUniformLocation(shader, "uGammaEffect");
    uShadeEffect = glGetUniformLocation(shader, "uShadeEffect");

    uRed = glGetUniformLocation(shader, "uRed");
    uGreen = glGetUniformLocation(shader, "uGreen");
    uInvis = glGetUniformLocation(shader, "uInvis");
    uGrey = glGetUniformLocation(shader, "uGrey");
    uInfra = glGetUniformLocation(shader, "uInfra");
    uWater = glGetUniformLocation(shader, "uWater");
    uShadow = glGetUniformLocation(shader, "uShadow");
    uUseInstancing = glGetUniformLocation(shader, "uUseInstancing");
    uModel = glGetUniformLocation(shader, "uModel");

    glUniformMatrix4fv(uProjection, 1, GL_FALSE, projection_matrix);

    glUseProgram(0);
}

void drop_effect_shader(void) {
    if (shader) {
        glDeleteProgram(shader);
        shader = 0;
    }
}

GLint use_effect_shader(EffectShaderSettings settings) {
    glUseProgram(shader);

    // Reset all effect uniforms to prevent bleeding from previous sprites
    glUniform1i(uUseInstancing, 0); // Disable instanced rendering mode
    glUniform1i(uTexture, 0);
    glUniform1f(uGammaScale, settings.gamma_scale);


    if (settings.uv0[0] == 0 && settings.uv1[0] == 0 && settings.uv1[0] == 0 && settings.uv1[1] == 0) {
        // UV should be implicitly 0,0 -> 1,1
        glUniform2f(uUV0, 0.0f, 0.0f);
        glUniform2f(uUV1, 1.0f, 1.0f);
    } else {
        // UV was explicitly defined
        glUniform2f(uUV0, settings.uv0[0], settings.uv0[1]);
        glUniform2f(uUV1, settings.uv1[0], settings.uv1[1]);
    }

    glUniform1f(uShadeEffect, settings.shade_effect);
    glUniform1f(uGammaEffect, settings.gamma_effect);
    glUniform1i(uRed, settings.red);
    glUniform1i(uGreen, settings.green);
    glUniform1i(uInvis, settings.invis);
    glUniform1i(uGrey, settings.grey);
    glUniform1i(uInfra, settings.infra);
    glUniform1i(uWater, settings.water);
    glUniform1i(uShadow, settings.shadow);

    return uModel;
}

void use_effect_shader_instanced(void) {
    glUseProgram(shader);
    glUniform1i(uTexture, 0); // Texture unit 0
    glUniform1i(uShadow, 0); // Ensure shadow uniform is off for batch
    glUniform1i(uUseInstancing, 1); // Enable instanced rendering mode
}

/* ===== ImGui-Compatible Effect Shader ===== */
/* This version uses vec2 position to match ImGui's vertex format */

static GLuint imgui_shader = 0;

static GLint imgui_uProjection = -1;
static GLint imgui_uModel = -1;
static GLint imgui_uTexture = -1;
static GLint imgui_uUV0 = -1;
static GLint imgui_uUV1 = -1;
static GLint imgui_uGammaScale = -1;
static GLint imgui_uGammaEffect = -1;
static GLint imgui_uShadeEffect = -1;
static GLint imgui_uRed = -1;
static GLint imgui_uGreen = -1;
static GLint imgui_uInvis = -1;
static GLint imgui_uGrey = -1;
static GLint imgui_uInfra = -1;
static GLint imgui_uWater = -1;
static GLint imgui_uShadow = -1;
static GLint imgui_uBuff = -1;

void load_effect_shader_imgui(float projection_matrix[16]) {
    imgui_shader = load_shader_program(effect_imgui_vertex, effect_imgui_fragment);
    if (!imgui_shader) {
        printf("Failed to load ImGui effect shader\n");
        return;
    }

    glUseProgram(imgui_shader);

    /* Cache uniform locations */
    imgui_uProjection = glGetUniformLocation(imgui_shader, "uProjection");
    imgui_uModel = glGetUniformLocation(imgui_shader, "uModel");
    imgui_uTexture = glGetUniformLocation(imgui_shader, "uTexture");
    imgui_uUV0 = glGetUniformLocation(imgui_shader, "uUV0");
    imgui_uUV1 = glGetUniformLocation(imgui_shader, "uUV1");
    imgui_uGammaScale = glGetUniformLocation(imgui_shader, "uGammaScale");
    imgui_uGammaEffect = glGetUniformLocation(imgui_shader, "uGammaEffect");
    imgui_uShadeEffect = glGetUniformLocation(imgui_shader, "uShadeEffect");
    imgui_uRed = glGetUniformLocation(imgui_shader, "uRed");
    imgui_uGreen = glGetUniformLocation(imgui_shader, "uGreen");
    imgui_uInvis = glGetUniformLocation(imgui_shader, "uInvis");
    imgui_uGrey = glGetUniformLocation(imgui_shader, "uGrey");
    imgui_uInfra = glGetUniformLocation(imgui_shader, "uInfra");
    imgui_uWater = glGetUniformLocation(imgui_shader, "uWater");
    imgui_uShadow = glGetUniformLocation(imgui_shader, "uShadow");
    imgui_uBuff = glGetUniformLocation(imgui_shader, "uBuff");

    glUniformMatrix4fv(uProjection, 1, GL_FALSE, projection_matrix);

    glUseProgram(0);
}

void drop_effect_shader_imgui(void) {
    if (imgui_shader) {
        glDeleteProgram(imgui_shader);
        imgui_shader = 0;
    }
}

void use_effect_shader_imgui(EffectShaderSettings settings) {
    glUseProgram(imgui_shader);

    /* Set identity model matrix (ImGui already handles positioning) */
    static const float identity[16] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
    glUniformMatrix4fv(imgui_uModel, 1, GL_FALSE, identity);

    /* Set texture sampler */
    glUniform1i(imgui_uTexture, 0);

    /* Set UV coordinates */
    if (settings.uv0[0] == 0 && settings.uv1[0] == 0 &&
        settings.uv1[0] == 0 && settings.uv1[1] == 0) {
        glUniform2f(imgui_uUV0, 0.0f, 0.0f);
        glUniform2f(imgui_uUV1, 1.0f, 1.0f);
    } else {
        glUniform2f(imgui_uUV0, settings.uv0[0], settings.uv0[1]);
        glUniform2f(imgui_uUV1, settings.uv1[0], settings.uv1[1]);
    }

    /* Set effect uniforms - same as non-instanced path */
    glUniform1f(imgui_uGammaScale, settings.gamma_scale);
    glUniform1f(imgui_uShadeEffect, settings.shade_effect);
    glUniform1f(imgui_uGammaEffect, settings.gamma_effect);
    glUniform1i(imgui_uRed, settings.red);
    glUniform1i(imgui_uGreen, settings.green);
    glUniform1i(imgui_uInvis, settings.invis);
    glUniform1i(imgui_uGrey, settings.grey);
    glUniform1i(imgui_uInfra, settings.infra);
    glUniform1i(imgui_uWater, settings.water);
    glUniform1i(imgui_uShadow, settings.shadow);
    glUniform1i(imgui_uBuff, 0);
}
