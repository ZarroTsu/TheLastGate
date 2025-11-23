#include <ddraw.h>

#include "DD.H"
#include "SDL.h"
#include "render.h"

#include <SDL2/SDL_image.h>

int init_windowed(HWND hwnd, int x, int y) {
    if (SDL_ENABLED && sdl_init())
        return -1;

    if (DD_ENABLED && dd_init_windowed(hwnd, x, y))
        return -1;

    return 0;
}

int init(HWND hwnd, int x, int y) {
    if (SDL_ENABLED && !sdl_init())
        return -1;

    if (SDL_ENABLED && !dd_init(hwnd, x, y))
        return -1;

    return 0;
}

void deinit_windowed(void) {
    if (SDL_ENABLED) sdl_deinit();
    if (DD_ENABLED) dd_deinit_windowed();
}

void deinit(void) {
    if (SDL_ENABLED) sdl_deinit();
    if (DD_ENABLED) dd_deinit();
}

void copysprite(int nr, int effect, int x, int y, int xoff, int yoff) {
    if (DD_ENABLED) dd_copysprite(nr, effect, x, y, xoff, yoff);
    if (SDL_ENABLED) sdl_copysprite(nr, effect, x, y, xoff, yoff);
}

void copyspritex(int nr, int xpos, int ypos, int effect) {
    if (DD_ENABLED) dd_copyspritex(nr, xpos, ypos, effect);
    if (SDL_ENABLED) sdl_copyspritex(nr, xpos, ypos, effect);
}

void puttext(int x, int y, int font, char *text) {
    if (DD_ENABLED) dd_puttext(x, y, font, text);
    if (SDL_ENABLED) sdl_puttext(x, y, font, text);
}

void xputtext(int x, int y, int font, char *format, ...) {
    va_list args1, args2;
    va_start(args1, format);

    va_copy(args2, args1);

    if (DD_ENABLED) dd_xputtext(x, y, font, format, args1);
    if (SDL_ENABLED) sdl_xputtext(x, y, font, format, args2);
    va_end(args1);
    va_end(args2);
}

void gputtext(int x, int y, int font, char *text, int xoff, int yoff) {
    if (DD_ENABLED) dd_gputtext(x, y, font, text, xoff, yoff);
    if (SDL_ENABLED) sdl_gputtext(x, y, font, text, xoff, yoff);
}

void render_putc(int x, int y, char font, int c) {
    if (DD_ENABLED) dd_putc(x, y, font, c);
    if (SDL_ENABLED) sdl_putc(x, y, font, c);
}

void showbox(int xf, int yf, int xs, int ys, unsigned short col) {
    if (DD_ENABLED) dd_showbox(xf, yf, xs, ys, col);
    if (SDL_ENABLED) sdl_showbox(xf, yf, xs, ys, col);
}

void showbar(int xf, int yf, int xs, int ys, unsigned short col) {
    if (DD_ENABLED) dd_showbar(xf, yf, xs, ys, col);
    if (SDL_ENABLED) sdl_showbar(xf, yf, xs, ys, col);
}

void shadow_clear(void) {
    if (DD_ENABLED) dd_shadow_clear();
    if (SDL_ENABLED) sdl_shadow_clear();
}

void shadow(int nr, int xpos, int ypos, int xoff, int yoff) {
    if (DD_ENABLED) dd_shadow(nr, xpos, ypos, xoff, yoff);
    if (SDL_ENABLED) sdl_shadow(nr, xpos, ypos, xoff, yoff);
}

void alphaeffect_magic(int nr, int str, int xpos, int ypos, int xoff, int yoff) {
    if (DD_ENABLED) dd_alphaeffect_magic(nr, str, xpos, ypos, xoff, yoff);
    if (SDL_ENABLED) sdl_alphaeffect_magic(nr, str, xpos, ypos, xoff, yoff);
}

void show_map(unsigned short *src, int xo, int yo, int magnify) {
    if (DD_ENABLED) dd_show_map(src, xo, yo, magnify);
    if (SDL_ENABLED) sdl_show_map(src, xo, yo, magnify);
}

int isvisible(void) {
    if (DD_ENABLED) return dd_isvisible();
    if (SDL_ENABLED) return sdl_isvisible();
}

int get_avgcol(int nr) {
    if (DD_ENABLED) return dd_get_avgcol(nr);
    if (SDL_ENABLED) return sdl_get_avgcol(nr);
}

SDL_Cursor* load_cursor_from_png(const char *filename, int hot_x, int hot_y) {
    SDL_Surface* surface = IMG_Load(filename);
    if (!surface) {
        printf("Failed to load cursor %s: %s\n", filename, IMG_GetError());
        return SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
    }

    SDL_Cursor* cursor = SDL_CreateColorCursor(surface, hot_x, hot_y);
    SDL_FreeSurface(surface);
    return cursor;
}
