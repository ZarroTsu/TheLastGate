#pragma once

#include <SDL2/SDL.h>

int init(int windowed);

void deinit(void);

void copysprite(int nr, int effect, int x, int y, int xoff, int yoff);

void copyspritex(int nr, int xpos, int ypos, int effect);

void puttext(int x, int y, int font, char *text);

void xputtext(int x, int y, int font, char *format, ...);

void gputtext(int x, int y, int font, char *text, int xoff, int yoff);

void render_putc(int x, int y, char font, int c);

void showbox(int xf, int yf, int xs, int ys, unsigned short col);

void showbar(int xf, int yf, int xs, int ys, unsigned short col);

void shadow_clear(void);

void shadow(int nr, int xpos, int ypos, int xoff, int yoff);

void alphaeffect_magic(int nr, int str, int xpos, int ypos, int xoff, int yoff);

void show_map(unsigned short *src, int xo, int yo, int magnify);

int isvisible(void);

int get_avgcol(int nr);

SDL_Cursor *load_cursor_from_png(const char *filename, int hot_x, int hot_y);
