#ifndef THELASTGATE_SDL_H
#define THELASTGATE_SDL_H
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

int sdl_init(void);

typedef struct {
    SDL_Renderer *renderer;
    SDL_Window *window;
} App;

extern App app;

#endif //THELASTGATE_SDL_H