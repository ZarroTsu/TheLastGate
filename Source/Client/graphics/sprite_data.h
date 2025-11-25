#ifndef THELASTGATE_SPRITE_DATA_H
#define THELASTGATE_SPRITE_DATA_H

#include "SDL2/SDL.h"

typedef struct {
    float u;
    float v;
} UV;

typedef struct {
    unsigned int gl_texture; // OpenGL texture ID
    SDL_Surface *surface;
    unsigned int gl_shadow_texture; // OpenGL shadow texture ID
    unsigned char xs; // Width in 32x32 tiles
    unsigned char ys; // Height in 32x32 tiles
    unsigned short alphacnt; // Number of bytes in alpha array
    unsigned short avgcol; // Average color (RGB565 format)
    int pixel_width; // Actual pixel width
    int pixel_height; // Actual pixel height
    bool loaded_in_atlas;
    UV uv0;
    UV uv1;
} SpriteData;

#endif //THELASTGATE_SPRITE_DATA_H