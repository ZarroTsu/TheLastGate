#include <ddraw.h>

#include "graphics/sdl.h"
#include "render.h"

#include <SDL2/SDL_image.h>

#include "graphics/loader.h"

int init(const int windowed) {
    if (init_image_loader() == -1) return -1;
    return sdl_init(windowed);
}

void deinit(void) {
    sdl_deinit();
}

void copysprite(int nr, int effect, int x, int y, int xoff, int yoff) {
    sdl_copysprite(nr, effect, x, y, xoff, yoff);
}

void copyspritex(int nr, int xpos, int ypos, int effect) {
    sdl_copyspritex(nr, xpos, ypos, effect);
}

void puttext(int x, int y, int font, char *text) {
    sdl_puttext(x, y, font, text);
}

void xputtext(int x, int y, int font, char *format, ...) {
    va_list args1;
    va_start(args1, format);
    sdl_xputtext(x, y, font, format, args1);
    va_end(args1);
}

void gputtext(int x, int y, int font, char *text, int xoff, int yoff) {
    sdl_gputtext(x, y, font, text, xoff, yoff);
}

void render_putc(int x, int y, char font, int c) {
    sdl_putc(x, y, font, c);
}

void showbox(int xf, int yf, int xs, int ys, unsigned short col) {
    sdl_showbox(xf, yf, xs, ys, col);
}

void showbar(int xf, int yf, int xs, int ys, unsigned short col) {
    sdl_showbar(xf, yf, xs, ys, col);
}

void shadow_clear(void) {
    sdl_shadow_clear();
}

void shadow(int nr, int xpos, int ypos, int xoff, int yoff) {
    sdl_shadow(nr, xpos, ypos, xoff, yoff);
}

void alphaeffect_magic(int nr, int str, int xpos, int ypos, int xoff, int yoff) {
    sdl_alphaeffect_magic(nr, str, xpos, ypos, xoff, yoff);
}

void show_map(unsigned short *src, int xo, int yo, int magnify) {
    sdl_show_map(src, xo, yo, magnify);
}

int isvisible(void) {
    return sdl_isvisible();
}

int get_avgcol(int nr) {
    return sdl_get_avgcol(nr);
}

SDL_Cursor *load_cursor_from_png(const char *filename, int hot_x, int hot_y) {
    SDL_Surface *surface = IMG_Load(filename);
    if (!surface) {
        printf("Failed to load cursor %s: %s\n", filename, IMG_GetError());
        return SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
    }

    SDL_Cursor *cursor = SDL_CreateColorCursor(surface, hot_x, hot_y);
    SDL_FreeSurface(surface);
    return cursor;
}
