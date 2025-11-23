#ifndef THELASTGATE_SDL_H
#define THELASTGATE_SDL_H
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "inter.h"
#include "main.h"
#include "render.h"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define MAX_SPRITES (2000 + (128 * 1024))
#define X_OFFSET (224+14)
#define Y_OFFSET 460
#define RENDER_DISTANCE 54 // Mush match server-side
#define CLAMP8(v) ((v) < 0 ? 0 : ((v) > 255 ? 255 : (v)))
#define LIGHT_EFFECT	(gamma-4880)   //120

typedef struct {
    SDL_Renderer *renderer;
    SDL_Window *window;
} App;

struct FontCache {
    SDL_Texture *char_textures[96];
};

extern App app;

int sdl_init(void);
void sdl_deinit(void);
void sdl_init_sprites(void);
void sdl_load_sprite(int nr);
void sdl_free_sprite(int nr);
void sdl_copysprite(int nr, int effect, int x, int y, int xoff, int yoff);
void sdl_copyspritex(int nr, int x, int y, int effect);
void sdl_puttext(int x, int y, int font, char *text);
void sdl_gputtext(int xpos,int ypos,int font,char *text,int xoff,int yoff);
void sdl_putc(int xpos, int ypos, int font, int c);
void sdl_xputtext(int x, int y, int font, char *format, va_list args);
void sdl_showbox(int xf,int yf,int xs,int ys,unsigned short col);
void sdl_showbar(int xf,int yf,int xs,int ys,unsigned short col);
void sdl_shadow(int nr,int xpos,int ypos,int xoff,int yoff);
void sdl_shadow_clear(void);
int sdl_isvisible(void);
void sdl_show_map(unsigned short *src,int xo,int yo,int magnify);
int sdl_get_avgcol(int nr);

// Magic glow effects (sdl_magic.c)
void sdl_init_magic_effects(void);
void sdl_deinit_magic_effects(void);
void sdl_alphaeffect_magic(int nr,int str,int xpos,int ypos,int xoff,int yoff);
void sdl_alphaeffect_magic_scaled(int nr, int str, int xpos, int ypos, int xoff, int yoff, float scale);

static void apply_effects(SDL_Surface *surface, int effect);
#endif //THELASTGATE_SDL_H
