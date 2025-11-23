/*
 * SDL2 Magic Glow Effect Demo
 * ============================
 * Demonstrates all magic glow effect colors and strengths.
 *
 * Compile:
 *   gcc -o magic_demo sdl_magic_demo.c sdl_magic.c -lSDL2 -lSDL2_image -lm
 *
 * Run:
 *   ./magic_demo
 *
 * Controls:
 *   - ESC: Exit
 *   - 1-7: Change effect color
 *   - +/-: Increase/decrease strength
 *   - Mouse: Move effect position
 */

#include "sdl.h"
#include <stdio.h>
#include <stdlib.h>

// Simplified app initialization for demo
App app;

int main(int argc, char *argv[]) {
    SDL_Event event;
    int running = 1;
    int current_color = 7;  // White glow
    int current_str = 1;    // Full intensity

    // Mouse position
    int mouse_x = SCREEN_WIDTH / 2;
    int mouse_y = SCREEN_HEIGHT / 2;

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL initialization failed: %s\n", SDL_GetError());
        return 1;
    }

    app.window = SDL_CreateWindow(
        "Magic Glow Effect Demo",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        0
    );

    if (!app.window) {
        printf("Window creation failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    app.renderer = SDL_CreateRenderer(
        app.window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (!app.renderer) {
        printf("Renderer creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(app.window);
        SDL_Quit();
        return 1;
    }

    // Initialize magic effects
    sdl_init_magic_effects();

    printf("Magic Glow Effect Demo\n");
    printf("======================\n");
    printf("Controls:\n");
    printf("  ESC - Exit\n");
    printf("  1   - Red glow\n");
    printf("  2   - Green glow\n");
    printf("  3   - Yellow glow (red+green)\n");
    printf("  4   - Blue glow\n");
    printf("  5   - Magenta glow (red+blue)\n");
    printf("  6   - Cyan glow (green+blue)\n");
    printf("  7   - White glow (all colors)\n");
    printf("  +/- - Adjust strength\n");
    printf("  Mouse - Move effect\n\n");

    // Main loop
    while (running) {
        // Handle events
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = 0;
                    break;

                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                        case SDLK_ESCAPE:
                            running = 0;
                            break;
                        case SDLK_1:
                            current_color = 1;  // Red
                            printf("Color: Red (fire/damage)\n");
                            break;
                        case SDLK_2:
                            current_color = 2;  // Green
                            printf("Color: Green (poison/nature)\n");
                            break;
                        case SDLK_3:
                            current_color = 3;  // Yellow
                            printf("Color: Yellow (holy magic)\n");
                            break;
                        case SDLK_4:
                            current_color = 4;  // Blue
                            printf("Color: Blue (ice/mana)\n");
                            break;
                        case SDLK_5:
                            current_color = 5;  // Magenta
                            printf("Color: Magenta (arcane)\n");
                            break;
                        case SDLK_6:
                            current_color = 6;  // Cyan
                            printf("Color: Cyan (water)\n");
                            break;
                        case SDLK_7:
                            current_color = 7;  // White
                            printf("Color: White (pure light)\n");
                            break;
                        case SDLK_EQUALS:
                        case SDLK_PLUS:
                            if (current_str > 1) {
                                current_str--;
                                printf("Strength: %d (stronger)\n", current_str);
                            }
                            break;
                        case SDLK_MINUS:
                            if (current_str < 8) {
                                current_str++;
                                printf("Strength: %d (weaker)\n", current_str);
                            }
                            break;
                    }
                    break;

                case SDL_MOUSEMOTION:
                    mouse_x = event.motion.x;
                    mouse_y = event.motion.y;
                    break;
            }
        }

        // Clear screen to dark background
        SDL_SetRenderDrawColor(app.renderer, 20, 20, 30, 255);
        SDL_RenderClear(app.renderer);

        // Draw a simple grid pattern to show blending
        SDL_SetRenderDrawColor(app.renderer, 40, 40, 50, 255);
        for (int x = 0; x < SCREEN_WIDTH; x += 32) {
            SDL_RenderDrawLine(app.renderer, x, 0, x, SCREEN_HEIGHT);
        }
        for (int y = 0; y < SCREEN_HEIGHT; y += 32) {
            SDL_RenderDrawLine(app.renderer, 0, y, SCREEN_WIDTH, y);
        }

        // Draw info text (simplified - would need font rendering)
        // For demo, we'll just show the effect

        // Render magic glow at mouse position
        // Convert screen coords to "world" coords for the function
        // (For demo purposes, we'll just use screen coords directly)
        int world_x = (mouse_x - X_OFFSET) * 2;
        int world_y = (mouse_y - Y_OFFSET) * 2;

        sdl_alphaeffect_magic(current_color, current_str, world_x, world_y, 0, 0);

        // Also render a grid of all effects for comparison
        for (int i = 1; i <= 7; i++) {
            int grid_x = 100 + (i - 1) * 150;
            int grid_y = 100;

            // Convert to world coords
            int wx = (grid_x - X_OFFSET) * 2;
            int wy = (grid_y - Y_OFFSET) * 2;

            // Render effect
            sdl_alphaeffect_magic(i, 1, wx, wy, 0, 0);
        }

        // Present
        SDL_RenderPresent(app.renderer);

        // Small delay to limit CPU usage
        SDL_Delay(16);  // ~60 FPS
    }

    // Cleanup
    sdl_deinit_magic_effects();
    SDL_DestroyRenderer(app.renderer);
    SDL_DestroyWindow(app.window);
    SDL_Quit();

    return 0;
}
