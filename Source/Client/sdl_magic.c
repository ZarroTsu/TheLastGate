/*
 * SDL2 Magic Glow Effect Implementation
 * ======================================
 * Implements the dd_alphaeffect_magic shader-based approach using
 * pre-rendered gradient textures with custom blending.
 *
 * This provides pixel-perfect recreation of the DirectDraw magic glow
 * effects using modern SDL2 rendering.
 */

#include "graphics/sdl.h"
#include "glad/glad.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// External declarations from sdl.c
extern GLuint gamma_shader;
extern GLint gamma_uProjection;
extern GLint gamma_uUV0;
extern GLint gamma_uUV1;
extern GLint gamma_uModel;
extern GLint gamma_uTexture;
extern GLint gamma_uGammaScale;
extern GLint gamma_uGammaEffect;
extern GLint gamma_uShadeEffect;
extern GLint gamma_uRed;
extern GLint gamma_uGreen;
extern GLint gamma_uInvis;
extern GLint gamma_uGrey;
extern GLint gamma_uInfra;
extern GLint gamma_uWater;
extern float projection_matrix[16];
extern int projection_initialized;

// Forward declare helper from sdl.c
extern void create_model_matrix(float *matrix, float x, float y, float width, float height);

// Magic effect gradient cache
typedef struct {
    unsigned int gradients[8];  // OpenGL texture IDs for each color combination (nr 0-7)
    int initialized;
} MagicEffectCache;

static MagicEffectCache magic_cache = {0};

// Maximum value helper
static inline int imax(int a, int b) {
    return (a > b) ? a : b;
}

/*
 * sdl_create_magic_gradient - Create a pre-rendered gradient texture (OpenGL)
 * @nr: Color flags (bit 0=red, bit 1=green, bit 2=blue)
 * @str: Strength divisor
 *
 * Creates a 64x64 gradient texture with the elliptical falloff pattern
 * from the original dd_alphaeffect_magic implementation.
 */
static unsigned int sdl_create_magic_gradient(int nr, int str) {
    // Create 64x64 surface for gradient
    SDL_Surface *surf = SDL_CreateRGBSurfaceWithFormat(
        0, 64, 64, 32, SPRITE_PIXEL_FORMAT
    );

    if (!surf) {
        printf("Failed to create magic gradient surface: %s\n", SDL_GetError());
        return 0;
    }

    SDL_LockSurface(surf);
    Uint32 *pixels = (Uint32 *)surf->pixels;
    int pitch = surf->pitch / 4;

    for (int y = 0; y < 64; y++) {
        for (int x = 0; x < 64; x++) {
            // Calculate elliptical gradient strength (matches dd.c algorithm)
            int e = 32;

            // Horizontal fade (symmetric around x=32)
            if (x < 32) e -= (32 - x);
            if (x > 31) e -= (x - 31);

            // Vertical fade (asymmetric)
            if (y < 16) e -= (16 - y);           // Gentle top fade
            if (y > 55) e -= (y - 55) * 2;       // Fast bottom fade

            if (e < 0) e = 0;
            e /= imax(1, str);

            // Calculate darkening amount based on enabled channels
            int e2 = 0;
            if (nr & 1) e2 += e;
            if (nr & 2) e2 += e;
            if (nr & 4) e2 += e;

            // Calculate glow color (scale 0-32 to 0-255)
            int r = 0, g = 0, b = 0;
            if (nr & 1) r = e * 8;  // Red channel
            if (nr & 2) g = e * 8;  // Green channel
            if (nr & 4) b = e * 8;  // Blue channel

            // Clamp to 0-255
            if (r > 255) r = 255;
            if (g > 255) g = 255;
            if (b > 255) b = 255;

            // Alpha controls the blend strength
            // Higher alpha = more visible effect
            int alpha = e * 8;
            if (alpha > 255) alpha = 255;

            pixels[y * pitch + x] = SDL_MapRGBA(surf->format, r, g, b, alpha);
        }
    }

    SDL_UnlockSurface(surf);

    // Create OpenGL texture from surface
    unsigned int gl_texture;
    glGenTextures(1, &gl_texture);
    glBindTexture(GL_TEXTURE_2D, gl_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 64,
                 0, GL_BGRA, GL_UNSIGNED_BYTE, surf->pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    SDL_FreeSurface(surf);

    if (!gl_texture) {
        printf("Failed to create magic gradient OpenGL texture\n");
        return 0;
    }

    return gl_texture;
}

/*
 * sdl_init_magic_effects - Initialize magic effect gradient cache
 *
 * Pre-renders all 8 gradient textures (one for each color combination).
 * Call this once during initialization.
 */
void sdl_init_magic_effects(void) {
    if (magic_cache.initialized) return;

    printf("Initializing magic effect gradients...\n");

    // Create gradients for all 8 color combinations
    // Use str=1 for full intensity (can be adjusted per-call later)
    for (int nr = 0; nr < 8; nr++) {
        magic_cache.gradients[nr] = sdl_create_magic_gradient(nr, 1);
        if (!magic_cache.gradients[nr]) {
            printf("Warning: Failed to create magic gradient for nr=%d\n", nr);
        }
    }

    magic_cache.initialized = 1;
    printf("Magic effect gradients initialized\n");
}

/*
 * sdl_deinit_magic_effects - Cleanup magic effect resources
 */
void sdl_deinit_magic_effects(void) {
    if (!magic_cache.initialized) return;

    for (int nr = 0; nr < 8; nr++) {
        if (magic_cache.gradients[nr]) {
            glDeleteTextures(1, &magic_cache.gradients[nr]);
            magic_cache.gradients[nr] = 0;
        }
    }

    magic_cache.initialized = 0;
}

/*
 * sdl_alphaeffect_magic - Render magic glow effect
 * @nr: Color flags (bit 0=red, bit 1=green, bit 2=blue)
 * @str: Strength divisor (higher = weaker effect)
 * @xpos: World X position (center of effect)
 * @ypos: World Y position (center of effect)
 * @xoff: Screen X offset
 * @yoff: Screen Y offset
 *
 * Renders a 64x64 pixel elliptical colored glow effect for magic spells.
 * Uses pre-rendered gradient textures with additive blending.
 *
 * Color combinations:
 *   nr=1: Red (fire, damage)
 *   nr=2: Green (poison, nature)
 *   nr=4: Blue (ice, mana)
 *   nr=3: Yellow (red+green, holy)
 *   nr=5: Magenta (red+blue, arcane)
 *   nr=6: Cyan (green+blue, water)
 *   nr=7: White (all channels, pure light)
 */
void sdl_alphaeffect_magic(int nr, int str, int xpos, int ypos, int xoff, int yoff) {
    // Ensure magic effects are initialized
    if (!magic_cache.initialized) {
        sdl_init_magic_effects();
    }

    // Validate nr parameter (0-7)
    if (nr < 0 || nr >= 8) return;
    if (!magic_cache.gradients[nr]) return;

    // Convert world coordinates to screen coordinates (isometric projection)
    // Same math as dd_alphaeffect_magic_1 in dd.c
    int rx = (xpos/2) + (ypos/2) - (2*16) + 32 + X_OFFSET -
             ((RENDER_DISTANCE-34)/2*32);

    if (xpos < 0 && (xpos & 1)) rx--;
    if (ypos < 0 && (ypos & 1)) rx--;

    int ry = (xpos/4) - (ypos/4) + Y_OFFSET - 2*32;

    if (xpos < 0 && (xpos & 3)) ry--;
    if (ypos < 0 && (ypos & 3)) ry++;

    rx += xoff;
    ry += yoff;

    sdl_start_scaling();
    // Render using OpenGL with additive blending for glow effect
    // Set additive blending: result = src + dst
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    glUseProgram(gamma_shader);
    glUniformMatrix4fv(gamma_uProjection, 1, GL_FALSE, projection_matrix);

    float model_matrix[16];
    create_model_matrix(model_matrix, rx, ry, 64, 64);
    glUniformMatrix4fv(gamma_uModel, 1, GL_FALSE, model_matrix);
    glUniform1i(gamma_uTexture, 0);

    // Adjust alpha based on strength parameter
    // str=1: full intensity, str=2: half intensity, etc.
    float alpha_scale = 1.0f / (float)imax(1, str);
    glUniform2f(gamma_uUV0, 0.0f, 0.0f);
    glUniform2f(gamma_uUV1, 1.0f, 1.0f);
    glUniform1f(gamma_uGammaScale, alpha_scale);
    glUniform1f(gamma_uGammaEffect, 0.0f);
    glUniform1f(gamma_uShadeEffect, 0.0f);

    // Reset all effect bools to prevent bleeding from previous sprites
    glUniform1i(gamma_uRed, 0);
    glUniform1i(gamma_uGreen, 0);
    glUniform1i(gamma_uInvis, 0);
    glUniform1i(gamma_uGrey, 0);
    glUniform1i(gamma_uInfra, 0);
    glUniform1i(gamma_uWater, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, magic_cache.gradients[nr]);
    glBindVertexArray(app.quad_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    // Restore standard alpha blending (set in sdl_init)
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    sdl_stop_scaling();
}

/*
 * sdl_alphaeffect_magic_multi - Render magic effect with dynamic strength
 * @nr: Color flags
 * @str: Strength divisor
 * @xpos: World X position
 * @ypos: World Y position
 * @xoff: Screen X offset
 * @yoff: Screen Y offset
 * @scale: Size multiplier (1.0 = 64x64, 2.0 = 128x128)
 *
 * Extended version that supports scaling the effect size.
 */
void sdl_alphaeffect_magic_scaled(int nr, int str, int xpos, int ypos,
                                   int xoff, int yoff, float scale) {
    if (!magic_cache.initialized) {
        sdl_init_magic_effects();
    }

    if (nr < 0 || nr >= 8) return;
    if (!magic_cache.gradients[nr]) return;

    // Same coordinate conversion
    int rx = (xpos/2) + (ypos/2) - (2*16) + 32 + X_OFFSET -
             ((RENDER_DISTANCE-34)/2*32);
    if (xpos < 0 && (xpos & 1)) rx--;
    if (ypos < 0 && (ypos & 1)) rx--;

    int ry = (xpos/4) - (ypos/4) + Y_OFFSET - 2*32;
    if (xpos < 0 && (xpos & 3)) ry--;
    if (ypos < 0 && (ypos & 3)) ry++;

    rx += xoff;
    ry += yoff;

    // Apply scale
    int scaled_size = (int)(64 * scale);

    // Center the scaled effect
    rx -= (scaled_size - 64) / 2;
    ry -= (scaled_size - 64) / 2;

    sdl_start_scaling();
    // Render using OpenGL with additive blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    glUseProgram(gamma_shader);
    glUniformMatrix4fv(gamma_uProjection, 1, GL_FALSE, projection_matrix);

    float model_matrix[16];
    create_model_matrix(model_matrix, rx, ry, scaled_size, scaled_size);
    glUniformMatrix4fv(gamma_uModel, 1, GL_FALSE, model_matrix);
    glUniform1i(gamma_uTexture, 0);

    float alpha_scale = 1.0f / (float)imax(1, str);
    glUniform1f(gamma_uGammaScale, alpha_scale);
    glUniform1f(gamma_uGammaEffect, 0.0f);
    glUniform1f(gamma_uShadeEffect, 0.0f);

    // Reset all effect bools to prevent bleeding from previous sprites
    glUniform1i(gamma_uRed, 0);
    glUniform1i(gamma_uGreen, 0);
    glUniform1i(gamma_uInvis, 0);
    glUniform1i(gamma_uGrey, 0);
    glUniform1i(gamma_uInfra, 0);
    glUniform1i(gamma_uWater, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, magic_cache.gradients[nr]);
    glBindVertexArray(app.quad_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    // Restore standard alpha blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    sdl_stop_scaling();
}
