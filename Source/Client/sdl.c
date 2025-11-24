//
// Created by james on 11/22/2025.
//
// SDL2 Sprite Loading System
// ==========================
// This file implements the SDL2 equivalent of the DirectDraw sprite loading
// and management system from dd.c. It provides:
//
// 1. Sprite Loading (sdl_load_sprite):
//    - Loads PNG/BMP from individual files or archives (pnglib.dat, bmplib)
//    - Converts to SDL_Texture with color key transparency
//    - Extracts alpha channel data for pixel-perfect blending
//    - Calculates average color for lighting effects
//    - Manages memory usage with LRU caching
//
// 2. Sprite Structure (SpriteData):
//    - SDL_Texture for hardware-accelerated rendering
//    - Alpha channel data for partial transparency
//    - Tile cache for effect-modified variants (lighting, etc.)
//    - Metadata: dimensions, average color, access time
//
// 3. Helper Functions:
//    - load_surface_from_file: Load PNG/BMP from disk
//    - load_surface_from_archive: Load from pnglib.dat using SDL_RWops
//    - load_surface_from_conv: Load from legacy bmplib format
//    - extract_alpha_channel: Extract partial transparency data
//    - calculate_average_color: For lighting effects
//
// 4. Memory Management:
//    - sdl_free_sprite: Free texture and associated data
//    - Track texture memory usage
//    - TODO: Implement LRU cache eviction (free_2nd_cache equivalent)
//
// Usage:
//    sdl_init();           // Initialize SDL2
//    sdl_init_sprites();   // Allocate sprite table
//    sdl_load_sprite(42);  // Load sprite #42
//    // ... render sprite ...
//    sdl_free_sprite(42);  // Free when done
//    sdl_deinit();         // Cleanup
//
#include "sdl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL_image.h>


#include "inter.h"
#include "main.h"

App app;

#define TILE 32
#define MAXEFFECT 1024

// SDL2 equivalent of sprtab structure
typedef struct {
    SDL_Texture *texture; // Main sprite texture
    SDL_Surface *original_surface;
    SDL_Surface *surface; // The RGB565 surface
    SDL_Texture *shadow_texture;
    unsigned char *alpha; // Alpha channel data (x,y,r,g,b,a format)
    SDL_Texture **cache_tiles; // Cache for effect-modified tile variants
    unsigned char xs; // Width in 32x32 tiles
    unsigned char ys; // Height in 32x32 tiles
    unsigned short alphacnt; // Number of bytes in alpha array
    unsigned int ticker; // Last access time (for LRU cache)
    unsigned short avgcol; // Average color (RGB565 format)
    int pixel_width; // Actual pixel width
    int pixel_height; // Actual pixel height
} SpriteData;


static struct FontCache font_cache[10];
static SpriteData *sprite_data = NULL;
static unsigned int usedmem = 0; // Texture memory usage tracking
static unsigned int maxmem = 0; // Maximum texture memory allowed
static unsigned int current_tick = 0; // Current tick counter for LRU

extern char path[];

extern void *conv_load(int nr, int *xs, int *ys);

extern FILE *load_pnglib(int nr);

int sdl_init(void) {
    const int rendererFlags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
    const int windowFlags = 0;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Couldn't initialize SDL: %s\n", SDL_GetError());
        return -1;
    }

    app.window = SDL_CreateWindow("Last Gate SDL",SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
                                  SCREEN_HEIGHT, windowFlags);

    if (!app.window) {
        printf("Failed to open %d x %d window: %s\n", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_GetError());
        return -1;
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");

    app.renderer = SDL_CreateRenderer(app.window, -1, rendererFlags);

    if (!app.renderer) {
        printf("Couldn't create renderer: %s\n", SDL_GetError());
        return -1;
    }

    for (int n = 1; n < 10; n++) {
        char path[256];
        sprintf(path, "resources/cursor%d.png", n);
        cursors[n] = load_cursor_from_png(path, 0, 0);
    }

    return 0;
}

void sdl_deinit(void) {
    if (app.renderer) {
        SDL_DestroyRenderer(app.renderer);
        app.renderer = NULL;
    }

    if (app.window) {
        SDL_DestroyWindow(app.window);
        app.window = NULL;
    }

    SDL_Quit();
}

/**
 * sdl_init_sprites - Initialize sprite management system
 *
 * Allocates sprite table and sets memory limits for texture caching.
 * Should be called after sdl_init().
 */
void sdl_init_sprites(void) {
    if (!sprite_data) {
        sprite_data = calloc(MAX_SPRITES, sizeof(SpriteData));
        if (!sprite_data) {
            printf("Failed to allocate sprite table\n");
            return;
        }
    }

    // Set max texture memory to system RAM minus 512MB
    // TODO: Query actual system RAM
    maxmem = 1024 * 1024 * 512; // 512MB default
    usedmem = 0;
    current_tick = 0;
}

/**
 * get_pixel - Read pixel from SDL_Surface at coordinates
 * @surface: Source surface
 * @x: X coordinate
 * @y: Y coordinate
 *
 * Returns: Pixel value as Uint32
 */
static Uint32 get_pixel(SDL_Surface *surface, int x, int y) {
    int bpp = surface->format->BytesPerPixel;
    Uint8 *p = (Uint8 *) surface->pixels + y * surface->pitch + x * bpp;

    switch (bpp) {
        case 1: return *p;
        case 2: return *(Uint16 *) p;
        case 3:
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
                return p[0] << 16 | p[1] << 8 | p[2];
            else
                return p[0] | p[1] << 8 | p[2] << 16;
        case 4: return *(Uint32 *) p;
        default: return 0;
    }
}

/**
 * calculate_average_color - Calculate average RGB color of surface
 * @surface: Source surface
 *
 * Returns: Average color in RGB565 format
 */
static unsigned short calculate_average_color(SDL_Surface *surface) {
    Uint32 total_r = 0, total_g = 0, total_b = 0;
    int pixel_count = 0;  // Count only non-transparent pixels

    SDL_LockSurface(surface);

    for (int y = 0; y < surface->h; y++) {
        for (int x = 0; x < surface->w; x++) {
            Uint8 r, g, b;
            Uint32 pixel = get_pixel(surface, x, y);
            SDL_GetRGB(pixel, surface->format, &r, &g, &b);

            // Skip magenta transparency pixels (255, 0, 255)
            if (r == 255 && g == 0 && b == 255) {
                continue;
            }

            total_r += r;
            total_g += g;
            total_b += b;
            pixel_count++;
        }
    }

    SDL_UnlockSurface(surface);

    // If all pixels were transparent, return black
    if (pixel_count == 0) return 0;

    // Calculate average
    Uint8 avg_r = total_r / pixel_count;
    Uint8 avg_g = total_g / pixel_count;
    Uint8 avg_b = total_b / pixel_count;

    // Convert to RGB555 (DirectDraw format)
    return ((avg_r >> 3) << 10) | ((avg_g >> 3) << 5) | (avg_b >> 3);
}

/**
 * extract_alpha_channel - Extract partially transparent pixels
 * @surface: Source surface with alpha channel
 * @alphacnt_ptr: Output parameter for alpha data byte count
 *
 * Scans surface for pixels with partial transparency (0 < alpha < 255)
 * and stores them in format: x, y, r, g, b, a (6 bytes per pixel)
 *
 * Returns: Allocated alpha data array, or NULL if no alpha pixels found
 */
static unsigned char *extract_alpha_channel(SDL_Surface *surface, int *alphacnt_ptr) {
    unsigned char *alpha = NULL;
    int alphacnt = 0;
    int alphamax = 0;

    // Check if surface has alpha channel
    if (!(surface->format->format == SDL_PIXELFORMAT_RGBA32 ||
          surface->format->format == SDL_PIXELFORMAT_ARGB32 ||
          surface->format->Amask != 0)) {
        *alphacnt_ptr = 0;
        return NULL;
    }

    SDL_LockSurface(surface);

    for (int y = 0; y < surface->h; y++) {
        for (int x = 0; x < surface->w; x++) {
            Uint8 r, g, b, a;
            Uint32 pixel = get_pixel(surface, x, y);
            SDL_GetRGBA(pixel, surface->format, &r, &g, &b, &a);

            // Store pixels with partial transparency
            if (a > 0 && a < 255) {
                // Grow buffer if needed
                if (alphacnt >= alphamax - 8) {
                    alphamax += 256;
                    alpha = realloc(alpha, alphamax);
                }

                alpha[alphacnt++] = (unsigned char) x;
                alpha[alphacnt++] = (unsigned char) y;
                alpha[alphacnt++] = r;
                alpha[alphacnt++] = g;
                alpha[alphacnt++] = b;
                alpha[alphacnt++] = a;
            }
        }
    }

    SDL_UnlockSurface(surface);

    // Shrink to actual size
    if (alpha && alphacnt > 0) {
        alpha = realloc(alpha, alphacnt);
    }

    *alphacnt_ptr = alphacnt;
    return alpha;
}

/**
 * load_surface_from_file - Load PNG or BMP from individual file
 * @nr: Sprite number
 * @width: Output parameter for width
 * @height: Output parameter for height
 *
 * Returns: SDL_Surface or NULL if not found
 */
static SDL_Surface *load_surface_from_file(int nr, int *width, int *height) {
    char buf[256];
    SDL_Surface *surface = NULL;

    // Try PNG file first
    sprintf(buf, "%sgfx\\%05d.png", path, nr);
    surface = IMG_Load(buf);

    if (!surface) {
        // Try BMP file
        sprintf(buf, "%sgfx\\%05d.bmp", path, nr);
        surface = SDL_LoadBMP(buf);
    }

    if (surface) {
        *width = surface->w;
        *height = surface->h;
    }

    return surface;
}

/**
 * load_surface_from_archive - Load PNG from pnglib.dat archive
 * @nr: Sprite number
 * @width: Output parameter for width
 * @height: Output parameter for height
 *
 * Returns: SDL_Surface or NULL if not found
 */
static SDL_Surface *load_surface_from_archive(int nr, int *width, int *height) {
    FILE *fp = load_pnglib(nr);
    if (!fp) return NULL;

    // Read entire PNG into memory buffer
    // Note: We need to know the size. For now, read until next sprite or EOF.
    // This is a simplified version - production code should use proper size tracking
    long start = ftell(fp);
    fseek(fp, 0, SEEK_END);
    long end = ftell(fp);
    fseek(fp, start, SEEK_SET); // Return to current position set by load_pnglib
    long size = end - start;

    // Limit read to reasonable PNG size (max 2MB per sprite)
    if (size > 2 * 1024 * 1024) size = 2 * 1024 * 1024;

    unsigned char *buffer = malloc(size);
    if (!buffer) return NULL;

    size_t read = fread(buffer, 1, size, fp);
    if (read == 0) {
        free(buffer);
        return NULL;
    }

    // Create SDL_RWops from memory buffer
    SDL_RWops *rw = SDL_RWFromMem(buffer, read);
    if (!rw) {
        free(buffer);
        return NULL;
    }

    // Load PNG from RWops
    SDL_Surface *surface = IMG_Load_RW(rw, 1); // 1 = auto-close RWops
    free(buffer);

    if (surface) {
        *width = surface->w;
        *height = surface->h;
    }

    return surface;
}

/**
 * load_surface_from_conv - Load BMP from conv_load (legacy format)
 * @nr: Sprite number
 * @width: Output parameter for width
 * @height: Output parameter for height
 *
 * Converts 16-bit RGB565 pixel buffer to SDL_Surface
 *
 * Returns: SDL_Surface or NULL if not found
 */

extern HANDLE heap;

static SDL_Surface *load_surface_from_conv(int nr, int *width, int *height) {
    int w, h;
    unsigned short *pixels = conv_load(nr, &w, &h);
    if (!pixels) return NULL;

    // Create 16-bit RGB565 surface with correct format
    SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(
        0, w, h, 16, SDL_PIXELFORMAT_RGB565
    );

    if (!surface) {
        HeapFree(heap, 0, pixels);
        return NULL;
    }

    // Copy 16-bit RGB565 pixel data
    SDL_LockSurface(surface);
    memcpy(surface->pixels, pixels, w * h * 2);
    SDL_UnlockSurface(surface);

    HeapFree(heap, 0, pixels);

    *width = w;
    *height = h;

    // Set color key for transparency on RGB565 surface
    // Magenta in RGB565: R=31 (5 bits), G=0 (6 bits), B=31 (5 bits)
    // RGB565 format: RRRRRGGGGGGBBBBB = 1111100000011111 = 0xF81F
    Uint32 colorkey565 = SDL_MapRGB(surface->format, 255, 0, 255);
    SDL_SetColorKey(surface, SDL_TRUE, colorkey565);

    // Convert to ARGB8888 for consistent rendering
    SDL_Surface *converted_surface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_ARGB8888, 0);
    SDL_FreeSurface(surface);

    return converted_surface;
}

/**
 * sdl_load_sprite - SDL2 equivalent of dd_load_sprite
 * @nr: Sprite index number (0-MAX_SPRITES)
 *
 * Loads sprite graphics from various sources with fallback chain:
 * 1. Individual PNG file (gfx/XXXXX.png)
 * 2. PNG library archive (pnglib.dat)
 * 3. Individual BMP file (gfx/XXXXX.bmp)
 * 4. BMP library archive via conv_load (bmplib)
 * 5. Fallback to sprite #35 if not found
 *
 * The loaded image is converted to SDL_Texture and stored in sprite_data[nr].
 * Alpha channel data is separated for pixel-perfect blending.
 * Transparency color key (magenta: 255,0,255) is set.
 */
void sdl_load_sprite(int nr) {
    if (nr < 0 || nr >= MAX_SPRITES) return;
    if (!sprite_data) sdl_init_sprites();

    // Already loaded?
    if (sprite_data[nr].texture) return;

    SDL_Surface *surface = NULL;
    int width = 0, height = 0;

    // Try to load from various sources
    surface = load_surface_from_file(nr, &width, &height);

    if (!surface) {
        surface = load_surface_from_archive(nr, &width, &height);
    }

    if (!surface) {
        surface = load_surface_from_conv(nr, &width, &height);
    }

    // Fallback to sprite #35 if not found
    if (!surface && nr != 35) {
        printf("Sprite %d not found, using fallback sprite #35\n", nr);
        surface = load_surface_from_file(35, &width, &height);

        if (!surface) {
            surface = load_surface_from_archive(35, &width, &height);
        }

        if (!surface) {
            surface = load_surface_from_conv(35, &width, &height);
        }
    }

    if (!surface) {
        printf("Failed to load sprite %d\n", nr);
        return;
    }

    // Set color key for transparency (magenta: RGB 255, 0, 255)
    Uint32 colorkey = SDL_MapRGB(surface->format, 255, 0, 255);
    SDL_SetColorKey(surface, SDL_TRUE, colorkey);

    // Convert to ARGB8888 format
    sprite_data[nr].surface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_ARGB8888, 0);
    sprite_data[nr].original_surface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_ARGB8888, 0);

    // Ensure color key is set on converted surfaces (magenta in ARGB8888)
    Uint32 colorkey_argb = SDL_MapRGB(sprite_data[nr].surface->format, 255, 0, 255);
    SDL_SetColorKey(sprite_data[nr].surface, SDL_TRUE, colorkey_argb);
    SDL_SetColorKey(sprite_data[nr].original_surface, SDL_TRUE, colorkey_argb);

    // Create texture from surface
    SDL_Texture *texture = SDL_CreateTexture(
        app.renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        sprite_data[nr].surface->w,
        sprite_data[nr].surface->h
    );

    if (!texture) {
        printf("Failed to create texture for sprite %d: %s\n", nr, SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }

    // Set blend mode for alpha transparency
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

    // Extract alpha channel for pixel-perfect blending
    int alphacnt = 0;
    unsigned char *alpha = extract_alpha_channel(surface, &alphacnt);

    // Calculate average color for lighting effects
    unsigned short avgcol = calculate_average_color(surface);

    // Store sprite data
    sprite_data[nr].texture = texture;
    sprite_data[nr].alpha = alpha;
    sprite_data[nr].alphacnt = alphacnt;
    sprite_data[nr].pixel_width = width;
    sprite_data[nr].pixel_height = height;
    sprite_data[nr].xs = (unsigned char) (width / TILE);
    sprite_data[nr].ys = (unsigned char) (height / TILE);
    sprite_data[nr].avgcol = avgcol;
    sprite_data[nr].ticker = current_tick;

    // Allocate tile cache for effect variants
    int cache_size = sprite_data[nr].xs * sprite_data[nr].ys * MAXEFFECT;
    sprite_data[nr].cache_tiles = calloc(cache_size, sizeof(SDL_Texture *));

    // Track memory usage (approximate)
    int texture_mem = width * height * 4; // Assume 32-bit RGBA
    usedmem += texture_mem;

    SDL_FreeSurface(surface);

    printf("Loaded sprite %d: %dx%d pixels (%dx%d tiles)\n",
           nr, width, height, sprite_data[nr].xs, sprite_data[nr].ys);
}

/**
 * sdl_free_sprite - Free sprite texture and associated data
 * @nr: Sprite index number
 */
void sdl_free_sprite(int nr) {
    if (nr < 0 || nr >= MAX_SPRITES) return;
    if (!sprite_data) return;

    SpriteData *sprite = &sprite_data[nr];

    // Free main texture
    if (sprite->texture) {
        SDL_DestroyTexture(sprite->texture);
        sprite->texture = NULL;

        // Subtract from memory usage
        int texture_mem = sprite->pixel_width * sprite->pixel_height * 4;
        if (usedmem >= texture_mem) {
            usedmem -= texture_mem;
        }
    }

    // Free alpha data
    if (sprite->alpha) {
        free(sprite->alpha);
        sprite->alpha = NULL;
    }

    // Free cached effect tiles
    if (sprite->cache_tiles) {
        int cache_size = sprite->xs * sprite->ys * MAXEFFECT;
        for (int i = 0; i < cache_size; i++) {
            if (sprite->cache_tiles[i]) {
                SDL_DestroyTexture(sprite->cache_tiles[i]);
            }
        }
        free(sprite->cache_tiles);
        sprite->cache_tiles = NULL;
    }

    // Reset metadata
    sprite->alphacnt = 0;
    sprite->xs = 0;
    sprite->ys = 0;
    sprite->pixel_width = 0;
    sprite->pixel_height = 0;
    sprite->avgcol = 0;
    sprite->ticker = 0;
}


void sdl_copysprite(int nr, int effect, int x, int y, int xoff, int yoff) {
    if (nr == 0) return;

    sdl_load_sprite(nr);
    if (!sprite_data[nr].texture) return;


    unsigned int xs = sprite_data[nr].xs;
    unsigned int ys = sprite_data[nr].ys;

    unsigned int rx = (x / 2) + (y / 2) - (xs * 16) + 32 + X_OFFSET - ((RENDER_DISTANCE - 34) / 2 * 32);
    if (x < 0 && x & 1) rx--;
    if (y < 0 && y & 1) rx--;
    unsigned int ry = (x / 4) - (y / 4) + Y_OFFSET - ys * 32;
    if (x < 0 && x & 3) ry--;
    if (y < 0 && y & 3) ry++;

    rx += xoff;
    ry += yoff;

    SDL_Rect dst;
    dst.x = rx;
    dst.y = ry;
    dst.w = sprite_data[nr].pixel_width;
    dst.h = sprite_data[nr].pixel_height;

    memcpy(sprite_data[nr].surface->pixels, sprite_data[nr].original_surface->pixels,
           sprite_data[nr].surface->pitch * sprite_data[nr].surface->h);

    apply_effects(sprite_data[nr].surface, effect);
    SDL_UpdateTexture(sprite_data[nr].texture, NULL, sprite_data[nr].surface->pixels, sprite_data[nr].surface->pitch);
    SDL_RenderCopy(app.renderer, sprite_data[nr].texture, NULL, &dst);

    // Convert Isometric/screen

    // RenderCopy
}

void sdl_copyspritex(int nr, int x, int y, int effect) {
    if (nr == 0) return;

    sdl_load_sprite(nr);
    if (!sprite_data[nr].texture) return;

    unsigned int xs = sprite_data[nr].xs;
    unsigned int ys = sprite_data[nr].ys;

    SDL_Rect dst;
    dst.x = x;
    dst.y = y;
    dst.w = sprite_data[nr].pixel_width;
    dst.h = sprite_data[nr].pixel_height;

    memcpy(sprite_data[nr].surface->pixels, sprite_data[nr].original_surface->pixels,
           sprite_data[nr].surface->pitch * sprite_data[nr].surface->h);

    apply_effects(sprite_data[nr].surface, effect);
    SDL_UpdateTexture(sprite_data[nr].texture, NULL, sprite_data[nr].surface->pixels, sprite_data[nr].surface->pitch);
    SDL_RenderCopy(app.renderer, sprite_data[nr].texture, NULL, &dst);
}

void sdl_putc(int xpos, int ypos, int font, int c) {
    if (c > 127 || c < 32) return;
    c -= 32;

    if (!font_cache[font].char_textures[c]) {
        int nr = 18100 + font;
        sdl_load_sprite(nr);

        SDL_Surface *char_surf = SDL_CreateRGBSurfaceWithFormat(0, 6, 9, 32, SDL_PIXELFORMAT_ARGB8888);
        SDL_LockSurface(sprite_data[nr].surface);
        SDL_LockSurface(char_surf);

        for (int y = 0; y < 9; y++) {
            Uint32 *src_row = (Uint32 *) sprite_data[nr].surface->pixels + (y + 1) * sprite_data[nr].surface->pitch / 4
                              + c * 6;
            Uint32 *dst_row = (Uint32 *) char_surf->pixels + y * char_surf->pitch / 4;
            memcpy(dst_row, src_row, 6 * sizeof(Uint32));
        }

        SDL_UnlockSurface(char_surf);
        SDL_UnlockSurface(sprite_data[nr].surface);

        font_cache[font].char_textures[c] = SDL_CreateTextureFromSurface(app.renderer, char_surf);
        SDL_SetTextureBlendMode(font_cache[font].char_textures[c], SDL_BLENDMODE_BLEND);
        SDL_FreeSurface(char_surf);
    }

    SDL_Rect dst = {.x = xpos, .y = ypos, .w = 6, .h = 9};
    SDL_RenderCopy(app.renderer, font_cache[font].char_textures[c], NULL, &dst);
}

void sdl_gputc(int xpos, int ypos, int font, int c) {
    if (c > 127 || c < 32) return;
    c -= 32;

    // Windowed mode?
    ypos += 4;
    if (!font_cache[font].char_textures[c]) {
        int nr = 18100 + font;
        sdl_load_sprite(nr);

        SDL_Surface *char_surf = SDL_CreateRGBSurfaceWithFormat(0, 6, 9, 32, SDL_PIXELFORMAT_ARGB8888);
        SDL_LockSurface(sprite_data[nr].surface);
        SDL_LockSurface(char_surf);

        for (int y = 0; y < 9; y++) {
            Uint32 *src_row = (Uint32 *) sprite_data[nr].surface->pixels + (y + 1) * sprite_data[nr].surface->pitch / 4
                              + c * 6;
            Uint32 *dst_row = (Uint32 *) char_surf->pixels + y * char_surf->pitch / 4;
            memcpy(dst_row, src_row, 6 * sizeof(Uint32));
        }

        SDL_UnlockSurface(char_surf);
        SDL_UnlockSurface(sprite_data[nr].surface);

        font_cache[font].char_textures[c] = SDL_CreateTextureFromSurface(app.renderer, char_surf);
        SDL_SetTextureBlendMode(font_cache[font].char_textures[c], SDL_BLENDMODE_BLEND);
        SDL_FreeSurface(char_surf);
    }

    SDL_Rect dst = {.x = xpos, .y = ypos, .w = 6, .h = 9};
    SDL_RenderCopy(app.renderer, font_cache[font].char_textures[c], NULL, &dst);
}

void sdl_puttext(int x, int y, int font, char *text) {
    while (*text) {
        sdl_putc(x, y, (char) font, *text);
        text++;
        x += 6;
    }
}

void sdl_gputtext(int x, int y, int font, char *text, int xoff, int yoff) {
    int text_length = strlen(text);

    unsigned int rx = (x / 2) + (y / 2) + 32 - ((text_length * 5) / 2) + X_OFFSET - ((RENDER_DISTANCE - 34) / 2 * 32);
    if (x < 0 && (x & 1)) rx--;
    if (y < 0 && (y & 1)) rx--;
    unsigned int ry = (x / 4) - (y / 4) + Y_OFFSET - 64;
    if (x < 0 && (x & 3)) ry--;
    if (y < 0 && (y & 3)) ry++;

    rx += xoff;
    ry += yoff;

    while (*text) {
        if (rx < 0 || rx > SCREEN_WIDTH - 7 || ry < 0 || ry > SCREEN_HEIGHT - 10) return;
        sdl_gputc(rx, ry, font, *text);
        text++;
        rx += 6;
    }
}

void sdl_xputtext(int x, int y, int font, char *format, va_list args) {
    char buf[1024];
    vsnprintf(buf, sizeof(buf), format, args);
    sdl_puttext(x, y, font, buf);
}

void sdl_showbox(int xf, int yf, int xs, int ys, unsigned short col) {
    Uint8 r, g, b;

    r = ((col & 0xF800) >> 11) << 3;
    g = ((col & 0x07E0) >> 5) << 2;
    b = ((col & 0x001F)) << 3;

    SDL_SetRenderDrawColor(app.renderer, r, g, b, 255);
    SDL_Rect rect = {
        .x = xf,
        .y = yf,
        .w = xs,
        .h = ys
    };

    SDL_RenderDrawRect(app.renderer, &rect);
}

void sdl_showbar(int xf, int yf, int xs, int ys, unsigned short col) {
    Uint8 r, g, b;

    r = ((col & 0xF800) >> 11) << 3;
    g = ((col & 0x07E0) >> 5) << 2;
    b = ((col & 0x001F)) << 3;


    SDL_SetRenderDrawColor(app.renderer, r, g, b, 255);
    SDL_Rect rect = {
        .x = xf,
        .y = yf,
        .w = xs,
        .h = ys
    };

    SDL_RenderFillRect(app.renderer, &rect);
}

void sdl_shadow(int nr, int xpos, int ypos, int xoff, int yoff) {
    // Check sprite range
    int ok = 0, disp = 14;
    if ((nr >= 2000 && nr < 16336) || nr > 17360) ok = 1;
    if (!ok) return;

    sdl_load_sprite(nr);
    if (!sprite_data[nr].texture) return;

    // Convert world coords to screen coords (same isometric math as copysprite)
    int xs = sprite_data[nr].xs;
    int ys = sprite_data[nr].ys;
    int rx = (xpos / 2) + (ypos / 2) - (xs * 16) + 32 + X_OFFSET -
             ((RENDER_DISTANCE - 34) / 2 * 32);
    if (xpos < 0 && (xpos & 1)) rx--;
    if (ypos < 0 && (ypos & 1)) rx--;
    int ry = (xpos / 4) - (ypos / 4) + Y_OFFSET - ys * 32;
    if (xpos < 0 && (xpos & 3)) ry--;
    if (ypos < 0 && (ypos & 3)) ry++;

    rx += xoff;
    ry += yoff;
    ry += ys * 32 - disp; // Offset shadow downward

    // Create compressed shadow texture (25% height, darkened)
    if (!sprite_data[nr].shadow_texture) {
        // Use original_surface which has color key properly applied
        SDL_LockSurface(sprite_data[nr].original_surface);

        int shadow_width = sprite_data[nr].pixel_width;
        int shadow_height = ys * 8; // Only 8 pixels per tile (25% of 32)

        // Create shadow surface (compressed height)
        SDL_Surface *shadow_surf = SDL_CreateRGBSurfaceWithFormat(
            0, shadow_width, shadow_height, 32, SDL_PIXELFORMAT_ARGB8888
        );
        SDL_LockSurface(shadow_surf);

        Uint32 *src_pixels = sprite_data[nr].original_surface->pixels;
        Uint32 *dst_pixels = shadow_surf->pixels;
        int src_pitch = sprite_data[nr].original_surface->pitch / 4;
        int dst_pitch = shadow_surf->pitch / 4;

        // Sample every 4th row of sprite to create compressed shadow
        // DirectDraw renders shadow bottom-to-top, creating vertical flip
        for (int shadow_y = 0; shadow_y < shadow_height; shadow_y++) {
            int sprite_y = shadow_y * 4; // Sample every 4th row
            int dst_y = (shadow_height - 1) - shadow_y; // Flip vertically

            for (int x = 0; x < shadow_width; x++) {
                Uint8 r, g, b, a;
                Uint32 src_pixel = src_pixels[sprite_y * src_pitch + x];
                SDL_GetRGBA(src_pixel, sprite_data[nr].original_surface->format,
                            &r, &g, &b, &a);

                // DirectDraw: if pixel not transparent, darken background
                // Binary decision: shadow or no shadow
                if (a == 0) {
                    // Fully transparent - no shadow here
                    dst_pixels[dst_y * dst_pitch + x] = 0;
                } else {
                    // Any opacity = cast shadow
                    // Shadow darkens to 50% brightness (DirectDraw: v>>=1)
                    // With BLEND mode: use black with 50% alpha
                    // result = background * (1 - 0.5) + black * 0.5 = background * 0.5
                    r = 0;
                    g = 0;
                    b = 0;
                    a = 128; // 50% transparency = 50% darkening

                    Uint32 shadow_pixel = SDL_MapRGBA(shadow_surf->format, r, g, b, a);

                    dst_pixels[dst_y * dst_pitch + x] = shadow_pixel;
                }
            }
        }

        SDL_UnlockSurface(shadow_surf);
        SDL_UnlockSurface(sprite_data[nr].original_surface);

        // Create texture from compressed, darkened shadow
        sprite_data[nr].shadow_texture =
                SDL_CreateTextureFromSurface(app.renderer, shadow_surf);

        // Use MOD blend mode to darken underlying pixels (multiply blend)
        // DirectDraw reads background pixel, halves brightness, writes back
        // SDL_BLENDMODE_MOD multiplies: result = src * dst
        // Since shadow is dark gray, this darkens the background
        SDL_SetTextureBlendMode(sprite_data[nr].shadow_texture,
                                SDL_BLENDMODE_BLEND);
        SDL_FreeSurface(shadow_surf);
    }

    // Render the compressed shadow texture
    SDL_Rect dst = {
        .x = rx,
        .y = ry,
        .w = sprite_data[nr].pixel_width,
        .h = ys * 8 // Shadow is only 8 pixels per tile (compressed!)
    };

    SDL_RenderCopy(app.renderer, sprite_data[nr].shadow_texture, NULL, &dst);
}

static unsigned char *shadow_map = NULL;

void sdl_shadow_clear(void) {
    if (!shadow_map) {
        shadow_map = calloc(SCREEN_WIDTH * SCREEN_HEIGHT, 1);
    } else {
        memset(shadow_map, 0, SCREEN_WIDTH * SCREEN_HEIGHT);
    }
}

/**
 * sdl_isvisible - Check if SDL window is visible and ready for rendering
 *
 * SDL2 EQUIVALENT OF dd_isvisible():
 * Unlike DirectDraw, SDL2 automatically handles video memory management
 * and surface restoration. Surfaces/textures never become "lost" in SDL2.
 * The driver manages video memory transparently, migrating it as needed.
 *
 * However, we still need to check if the window is in a state where
 * rendering makes sense:
 *
 * WINDOW STATES TO CHECK:
 * - SDL_WINDOW_MINIMIZED: Window is minimized to taskbar/dock
 *   → Skip rendering (not visible to user, waste of CPU/GPU)
 *
 * - SDL_WINDOW_HIDDEN: Window is explicitly hidden via SDL_HideWindow()
 *   → Skip rendering (window intentionally hidden)
 *
 * STATES NOT CHECKED:
 * - SDL_WINDOW_SHOWN: Default state, checked implicitly (not hidden/minimized)
 * - SDL_WINDOW_MAXIMIZED: Still visible, render normally
 * - SDL_WINDOW_INPUT_FOCUS: Not relevant (can render without focus)
 * - SDL_WINDOW_MOUSE_FOCUS: Not relevant (can render without mouse over it)
 *
 * PERFORMANCE NOTE:
 * On some platforms, SDL may pause rendering when minimized automatically.
 * This check provides explicit control and avoids wasted CPU cycles.
 *
 * USAGE:
 * Called at the start of each frame (engine.c:show_screen()) before rendering:
 *   if (!dd_isvisible()) return;  // DirectDraw version
 *   if (!sdl_isvisible()) return; // SDL2 version
 *
 * Returns: 1 if window is visible and ready for rendering
 *          0 if window is minimized/hidden (skip rendering)
 */
int sdl_isvisible(void) {
    if (!app.window) return 0;

    Uint32 flags = SDL_GetWindowFlags(app.window);

    // Skip rendering if minimized or hidden
    if (flags & (SDL_WINDOW_MINIMIZED | SDL_WINDOW_HIDDEN)) {
        return 0;
    }

    return 1;
}

// This has to be here because I can't include dd.h cause im lazy
extern int gamma;

void apply_effects(SDL_Surface *surface, int effect) {
    int invis = 0, tmp, grey = 0, infra = 0, water = 0, bloody = 0, red = 0, green = 0;

    if (effect & 16) {
        effect -= 16;
        red = 1;
    } //red border
    if (effect & 32) {
        effect -= 32;
        green = 1;
    } //green border
    if (effect & 64) {
        effect -= 64;
        invis = 1;
    } //blackened out
    if (effect & 128) {
        effect -= 128;
        grey = 1;
    } //grey scale
    if (effect & 256) {
        effect -= 256;
        infra = 1;
    } //grey scale
    if (effect & 512) {
        effect -= 512;
        water = 1;
    } //grey scale
    //	if (effect&1024) { effect-=1024; bloody=1; } //grey scale

    int w = surface->w;
    int h = surface->h;

    Uint32 *pixels = surface->pixels;

    int pitch = surface->pitch / 4;

    for (int y = 0; y < h; ++y) {
        Uint32 *row = pixels + y * pitch;

        for (int x = 0; x < w; ++x) {
            Uint32 px = row[x];

            Uint8 r;
            Uint8 g;
            Uint8 b;
            Uint8 a;

            SDL_GetRGBA(px, surface->format, &r, &g, &b, &a);

            if (a == 0) {
                continue;
            }


            // Use int for intermediate calculations to prevent overflow/truncation
            int ri = r, gi = g, bi = b;

            if (effect) {
                ri = (ri * LIGHT_EFFECT) / (effect * effect + LIGHT_EFFECT);
                gi = (gi * LIGHT_EFFECT) / (effect * effect + LIGHT_EFFECT);
                bi = (bi * LIGHT_EFFECT) / (effect * effect + LIGHT_EFFECT);
            }

            if (grey) {
                int tmp = (ri + (gi / 2) + bi) / 6;
                ri = tmp;
                gi = tmp * 2;
                bi = tmp;
            }

            if (infra) {
                int tmp = (ri + (gi / 2) + bi) / 3;
                ri = tmp;
                gi = 0;
                bi = 0;
            }

            if (water) {
                int tmp = (ri + (gi / 2) + bi) / 2;
                tmp = CLAMP8(tmp);

                ri = (ri + tmp) / 3;
                gi = ((gi + tmp) / 3) * 2;
                bi = bi + tmp;
            }

            if (gamma != 5000) {
                float gamma_scale = gamma / 5000.0f;
                ri = (int) (ri * gamma_scale);
                gi = (int) (gi * gamma_scale);
                bi = (int) (bi * gamma_scale);
            }

            if (red) {
                ri *= 2;
                gi *= 2;
                bi *= 2;
            }

            if (green) {
                gi += 128;
            }

            if (invis) {
                ri = gi = bi = 0;
            }

            // Clamp all values to 0-255 range before converting back to Uint8
            r = CLAMP8(ri);
            g = CLAMP8(gi);
            b = CLAMP8(bi);

            row[x] = SDL_MapRGBA(surface->format, r, g, b, a);
        }
    }
}

static SDL_Texture *minimap_texture = NULL;

void sdl_show_map(unsigned short *src, int xo, int yo, int magnify) {
    // Create texture if needed (once) - use ARGB8888 to avoid RGB555/565 issues
    if (!minimap_texture) {
        minimap_texture = SDL_CreateTexture(
            app.renderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            128, 128
        );
    }

    // Lock texture for pixel access
    void *pixels;
    int pitch;
    SDL_LockTexture(minimap_texture, NULL, &pixels, &pitch);
    Uint32 *dst = (Uint32 *) pixels;
    int dst_pitch = pitch / 4; // Convert bytes to 32-bit pixels

    // Sample and copy pixels from source map, converting RGB555/565 to ARGB8888
    for (int x = 0; x < 128; x++) {
        int src_x = (x / magnify) + xo;
        for (int y = 0; y < 128; y++) {
            int src_y = (y / magnify) + yo;

            // Clamp to map bounds
            if (src_x < 0 || src_x >= MAPX_MAX ||
                src_y < 0 || src_y >= MAPY_MAX) {
                dst[y * dst_pitch + x] = 0xFF000000; // Black (opaque)
                continue;
            }

            // Read from source (transposed indexing: src[x_coord * MAPX_MAX + y_coord])
            unsigned short pixel555 = src[src_x * MAPX_MAX + src_y];

            // Convert RGB555 to RGB888 (DirectDraw typically uses RGB555)
            // RGB555: 0RRRRRGGGGGBBBBB (high bit unused)
            Uint8 r = ((pixel555 >> 10) & 0x1F) << 3; // 5 bits -> 8 bits
            Uint8 g = ((pixel555 >> 5) & 0x1F) << 3;  // 5 bits -> 8 bits
            Uint8 b = (pixel555 & 0x1F) << 3;         // 5 bits -> 8 bits

            // Expand to full 8-bit range (replicate high bits to low bits)
            r |= (r >> 5);  // Fill lower 3 bits
            g |= (g >> 5);  // Fill lower 3 bits
            b |= (b >> 5);  // Fill lower 3 bits

            dst[y * dst_pitch + x] = (0xFF << 24) | (r << 16) | (g << 8) | b;
        }
    }

    SDL_UnlockTexture(minimap_texture);

    // Render to screen (top-right corner)
    SDL_Rect dst_rect = {
        .x = 6,
        .y = 582, // Add +4 if windowed mode
        .w = 128,
        .h = 128
    };
    SDL_RenderCopy(app.renderer, minimap_texture, NULL, &dst_rect);
}

int sdl_get_avgcol(int nr) {
    return sprite_data[nr].avgcol;
}
