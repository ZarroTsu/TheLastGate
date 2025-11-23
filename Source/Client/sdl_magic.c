/*
 * SDL2 Magic Glow Effect Implementation
 * ======================================
 * Implements the dd_alphaeffect_magic shader-based approach using
 * pre-rendered gradient textures with custom blending.
 *
 * This provides pixel-perfect recreation of the DirectDraw magic glow
 * effects using modern SDL2 rendering.
 */

#include "sdl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Magic effect gradient cache
typedef struct {
    SDL_Texture *gradients[8];  // One for each color combination (nr 0-7)
    int initialized;
} MagicEffectCache;

static MagicEffectCache magic_cache = {0};

// Maximum value helper
static inline int imax(int a, int b) {
    return (a > b) ? a : b;
}

/*
 * sdl_create_magic_gradient - Create a pre-rendered gradient texture
 * @nr: Color flags (bit 0=red, bit 1=green, bit 2=blue)
 * @str: Strength divisor
 *
 * Creates a 64x64 gradient texture with the elliptical falloff pattern
 * from the original dd_alphaeffect_magic implementation.
 */
static SDL_Texture *sdl_create_magic_gradient(int nr, int str) {
    // Create 64x64 surface for gradient
    SDL_Surface *surf = SDL_CreateRGBSurfaceWithFormat(
        0, 64, 64, 32, SDL_PIXELFORMAT_ARGB8888
    );

    if (!surf) {
        printf("Failed to create magic gradient surface: %s\n", SDL_GetError());
        return NULL;
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

    // Create texture from surface
    SDL_Texture *texture = SDL_CreateTextureFromSurface(app.renderer, surf);
    SDL_FreeSurface(surf);

    if (!texture) {
        printf("Failed to create magic gradient texture: %s\n", SDL_GetError());
        return NULL;
    }

    // Use additive blending for glow effect
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_ADD);

    return texture;
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
            SDL_DestroyTexture(magic_cache.gradients[nr]);
            magic_cache.gradients[nr] = NULL;
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

    // Adjust alpha based on strength parameter
    // str=1: full intensity (255)
    // str=2: half intensity (128)
    // str=4: quarter intensity (64)
    Uint8 alpha_mod = 255 / imax(1, str);
    SDL_SetTextureAlphaMod(magic_cache.gradients[nr], alpha_mod);

    // Render the gradient texture
    SDL_Rect dst = {
        .x = rx,
        .y = ry,
        .w = 64,
        .h = 64
    };

    SDL_RenderCopy(app.renderer, magic_cache.gradients[nr], NULL, &dst);

    // Restore full alpha for next render
    SDL_SetTextureAlphaMod(magic_cache.gradients[nr], 255);
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

    Uint8 alpha_mod = 255 / imax(1, str);
    SDL_SetTextureAlphaMod(magic_cache.gradients[nr], alpha_mod);

    SDL_Rect dst = {
        .x = rx,
        .y = ry,
        .w = scaled_size,
        .h = scaled_size
    };

    SDL_RenderCopy(app.renderer, magic_cache.gradients[nr], NULL, &dst);
    SDL_SetTextureAlphaMod(magic_cache.gradients[nr], 255);
}
