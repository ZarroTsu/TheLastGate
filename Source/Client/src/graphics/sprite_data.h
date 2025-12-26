#pragma once

#include "SDL2/SDL.h"
#include <stdbool.h>

typedef struct {
    float u;
    float v;
} UV;

typedef struct {
    unsigned int gl_texture; // OpenGL texture ID
    unsigned int atlas_texture; // This is specifically the atlas texture this image is stored in. Technically the same as above for now.
    SDL_Surface *surface;
    unsigned int gl_shadow_texture; // OpenGL shadow texture ID
    unsigned char xs; // Width in 32x32 tiles
    unsigned char ys; // Height in 32x32 tiles
    unsigned short alphacnt; // Number of bytes in alpha array
    unsigned short avgcol; // Average color (RGB565 format)
    int pixel_width; // Actual pixel width
    int pixel_height; // Actual pixel height
    bool loaded_in_atlas;
    bool is_ui;
    UV uv0;
    UV uv1;
} SpriteData;
