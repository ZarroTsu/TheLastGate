//
// Created by james on 11/22/2025.
//
#include "sdl.h"

App app;

int sdl_init(void) {
    const int rendererFlags = SDL_RENDERER_ACCELERATED;
    const int windowFlags = 0;

    printf("init sdl");
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Couldn't initialize SDL: %s\n", SDL_GetError());
        return -1;
    }

    app.window = SDL_CreateWindow("Last Gate SDL",SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH,
                                  SCREEN_HEIGHT, windowFlags);

    if (!app.window) {
        printf("Failed to open %d x %d window: %s\n", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_GetError());
        return -1;
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

    app.renderer = SDL_CreateRenderer(app.window, -1, rendererFlags);

    if (!app.renderer) {
        printf("Couldn't create renderer: %s\n", SDL_GetError());
        return -1;
    }

    return 0;
}
