#ifndef THELASTGATE_SDL_H
#define THELASTGATE_SDL_H
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define MAX_SPRITES (2000 + (128 * 1024))
#define X_OFFSET (224+14)
#define Y_OFFSET 460
#define RENDER_DISTANCE 54 // Mush match server-side
#define CLAMP8(v) ((v) < 0 ? 0 : ((v) > 255 ? 255 : (v)))
#define GAMMA_EFFECT	(app_state.gamma-4880)   //120

// Maximum sprites that can be batched in a single draw call
#define MAX_BATCH_SIZE 10000

// Per-sprite instance data for batched rendering
typedef struct {
    float model[16];      // Model matrix (position + size)
    float uv0[2];         // Top-left UV coordinate
    float uv1[2];         // Bottom-right UV coordinate
    float gamma_scale;    // Gamma correction scale
    float shade_effect;   // Lighting effect strength
    float gamma_effect;   // Lighting effect gamma
    int flags;            // Packed effect flags (red, green, invis, grey, infra, water, shadow)
} SpriteInstance;

typedef struct {
    SDL_Window *window;
    SDL_GLContext gl_context;
    unsigned int quad_vao;     // Vertex Array Object for sprite quads
    unsigned int quad_vbo;     // Vertex Buffer Object for sprite quads
    unsigned int instance_vbo; // Instance buffer for batched rendering
    SpriteInstance *batch_buffer; // CPU-side batch buffer
    int batch_count;              // Number of sprites in current batch
} App;

struct FontCache {
    unsigned int char_textures[96];  // OpenGL texture IDs (GLuint)
};

#define SPRITE_PIXEL_FORMAT SDL_PIXELFORMAT_ARGB8888

extern App app;

int sdl_init(int windowed);
void sdl_deinit(void);
void sdl_batch_begin(void);
void sdl_batch_flush(void);
void sdl_init_sprites(void);
void sdl_load_sprite(int nr);
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
void sdl_start_scaling(void);
void sdl_stop_scaling(void);

// Magic glow effects (sdl_magic.c)
void sdl_init_magic_effects(void);
void sdl_deinit_magic_effects(void);
void sdl_alphaeffect_magic(int nr,int str,int xpos,int ypos,int xoff,int yoff);
void sdl_alphaeffect_magic_scaled(int nr, int str, int xpos, int ypos, int xoff, int yoff, float scale);
#endif //THELASTGATE_SDL_H