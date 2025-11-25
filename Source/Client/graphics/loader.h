#ifndef THELASTGATE_LOADER_H
#define THELASTGATE_LOADER_H

#include <SDL2/SDL_surface.h>

#define MAX_PNG_SIZE (2 * 1024 * 1024)
#define LOADED_SPRITE_PIXEL_FORMAT SDL_PIXELFORMAT_ARGB8888

/**
 * All the functionality for loading images will go here, the only things out of this api
 * should be SDL_Surfaces.
 *
 * I'll leave conversion to anything else to the caller.
 */

int init_image_loader(void);
SDL_Surface *load_from_png_lib(int nr);
SDL_Surface *load_from_gfx_lib(int nr);
SDL_Surface *load_from_file(int nr);

typedef struct {
    int xs, ys;
    int off;
} Gfx;

#endif //THELASTGATE_LOADER_H
