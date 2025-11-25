//
// Created by james on 11/24/2025.
//

#include "loader.h"

#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <SDL2/SDL_image.h>

#include "sdl.h"
#include "../inter.h"
#include "../log.h"

// TODO: I don't want externs everywhere :(
extern char path[256];

static FILE *png_lib = nullptr;
static int png_index[MAXSPRITE];
static Gfx *gfx_lib;
static int gfx_handle;

static int init_png_lib(void);

static int init_gfx_lib(void);

static SDL_Surface *make_standard_format(SDL_Surface *loaded);

/**
 * Initialize the gfx*.dat, and the pnglib.dat
 *
 * @returns -1 (failed) or 0 (success)
 */
int init_image_loader() {
    if (init_png_lib() == -1) return -1;
    if (init_gfx_lib() == -1) return -1;
    return 0;
}

SDL_Surface *load_from_png_lib(const int nr) {
    if (nr < 0 || nr >= MAXSPRITE) return nullptr;
    const int offset = png_index[nr];
    if (!offset) return nullptr;
    fseek(png_lib, offset, SEEK_SET);

    const long start = ftell(png_lib);
    fseek(png_lib, 0, SEEK_END);
    const long end = ftell(png_lib);
    fseek(png_lib, start, SEEK_SET);
    long size = end - start;

    if (size > MAX_PNG_SIZE) {
        LOG("Attempt to load a too big sprite: nr=%d", nr);
        size = MAX_PNG_SIZE;
    }

    unsigned char *buffer = malloc(size);
    if (!buffer) return nullptr;

    size_t r = fread(buffer, 1, size, png_lib);
    if (r == 0) {
        free(buffer);
        return nullptr;
    }

    SDL_RWops *rw = SDL_RWFromMem(buffer, (int) r);
    if (!rw) {
        free(buffer);
        return nullptr;
    }

    SDL_Surface *surface = IMG_Load_RW(rw, 1);
    free(buffer);
    return make_standard_format(surface);
}

SDL_Surface *load_from_gfx_lib(const int nr) {
    if (nr >= 40000 || gfx_lib[nr].xs == 0) return nullptr;
    if (gfx_lib[nr].xs < 0 || gfx_lib[nr].xs > 1000 || gfx_lib[nr].ys < 0 || gfx_lib[nr].ys > 1000) {
        LOG("Graphics file gx00.idx corrupt!");
        return nullptr;
    }
    const ssize_t length = gfx_lib[nr].xs * gfx_lib[nr].ys * 2;
    unsigned short *buffer = malloc(length);
    if (!buffer) {
        LOG("Error allocating memory: length=%lld", length);
        free(buffer);
        return nullptr;
    }

    lseek(gfx_handle, gfx_lib[nr].off, SEEK_SET);
    read(gfx_handle, buffer, length);
    SDL_Surface *surface =
            SDL_CreateRGBSurfaceWithFormat(0, gfx_lib[nr].xs, gfx_lib[nr].ys, 16, SDL_PIXELFORMAT_RGB565);

    if (!surface) {
        free(buffer);
        return nullptr;
    }

    SDL_LockSurface(surface);
    memcpy(surface->pixels, buffer, length);
    SDL_UnlockSurface(surface);
    free(buffer);

    return make_standard_format(surface);
}

SDL_Surface *load_from_file(int nr) {
    char name[256];
    sprintf(name, "%sgfx\\%05d.png", path, nr);
    SDL_Surface *surface = IMG_Load(name);
    if (!surface) {
        sprintf(name, "%sgfx\\%05d.bmp", path, nr);
        surface = SDL_LoadBMP(name);
    }

    return make_standard_format(surface);
}

static int init_png_lib() {
    char file[80];

    snprintf(file, sizeof(file), "%spnglib.idx", path);
    const int handle = open(file, O_RDONLY | O_BINARY);
    if (handle == -1) {
        LOG("Could not open pnglib.idx: path=%s", file);
        return -1;
    }

    for (int i = 0; i < MAXSPRITE; i++) {
        const ssize_t r = read(handle, &png_index[i], sizeof(png_index[i]));
        if (r != sizeof(png_index[i])) { break; } // Nothing left to read
    }

    snprintf(file, sizeof(file), "%spnglib.dat", path);
    png_lib = fopen(file, "rb");
    if (png_lib == nullptr) {
        LOG("Could not open pnglib.dat: path=%s", file);
        return -1;
    }

    return 0;
}

static int init_gfx_lib() {
    char file[256];
    snprintf(file, sizeof(file), "%sgx00.idx", path);
    const int handle = open(file, O_RDONLY | O_BINARY);
    if (handle == -1) {
        LOG("Could not open gx00.idx: path=%s", file);
        return -1;
    }

    const int length = filelength(handle);
    gfx_lib = malloc(length);
    if (!gfx_lib) {
        LOG("Error allocating gfx_lib: lenth=%d", length);
        return -1;
    }

    if (length < sizeof(*gfx_lib) * 40000) {
        LOG("Graphics file gx00.idx corrupt!");
        return -1;
    }

    read(handle, gfx_lib, length);
    close(handle);

    sprintf(file, "%sgx00.dat", path);
    gfx_handle = open(file, O_RDONLY | O_BINARY);
    if (gfx_handle == -1) {
        LOG("Could not open gx00.dat: path=%s", file);
        return -1;
    }
    return 0;
}

static SDL_Surface *make_standard_format(SDL_Surface *loaded) {
    if (!loaded) return nullptr;

    unsigned int magenta = SDL_MapRGB(loaded->format, 255, 0, 255);
    SDL_SetColorKey(loaded, SDL_TRUE, magenta);
    SDL_Surface *converted_surface = SDL_ConvertSurfaceFormat(loaded, LOADED_SPRITE_PIXEL_FORMAT, 0);
    SDL_FreeSurface(loaded);
    if (!converted_surface) {
        LOG("make_standard_format failed: %s", SDL_GetError());
        return nullptr;;
    }

    return converted_surface;
}
