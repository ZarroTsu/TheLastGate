#include "input.h"

#include <SDL_syswm.h>

#include "engine.h"
#include "main.h"
#include "config/config.h"
#include "game/game_input.h"
#include "graphics/scaling.h"
#include "graphics/sdl.h"
#include "launcher/launcher_input.h"

SDL_Event input_events[128];
int input_event_count = 0;
bool window_resetting = false;

static void scale_mouse_event(SDL_Event *event) {
    /* Scale mouse coordinates from window space to virtual 1280x720 space for ImGui */
    if (g_config.video.window_size[0] == SCREEN_WIDTH && g_config.video.window_size[1] == SCREEN_HEIGHT) {
        return; /* No scaling needed */
    }

    float scaleX = (float) SCREEN_WIDTH / (float) g_config.video.window_size[0];
    float scaleY = (float) SCREEN_HEIGHT / (float) g_config.video.window_size[1];

    switch (event->type) {
        case SDL_MOUSEMOTION:
            event->motion.x = (int) (event->motion.x * scaleX);
            event->motion.y = (int) (event->motion.y * scaleY);
            break;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            event->button.x = (int) (event->button.x * scaleX);
            event->button.y = (int) (event->button.y * scaleY);
            break;
        case SDL_MOUSEWHEEL:
            event->wheel.mouseX = (int) (event->wheel.mouseX * scaleX);
            event->wheel.mouseY = (int) (event->wheel.mouseY * scaleY);
            break;
    }
}

#ifdef _WIN32

static void MakeWindowTopMost(SDL_Window *win) {
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);

    if (SDL_GetWindowWMInfo(win, &info)) {
        HWND hwnd = info.info.win.window;

        SetWindowPos(hwnd, HWND_TOPMOST,
                     0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }
}

static void MakeWindowNormal(SDL_Window *win) {
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);

    if (SDL_GetWindowWMInfo(win, &info)) {
        HWND hwnd = info.info.win.window;

        SetWindowPos(hwnd, HWND_NOTOPMOST,
                     0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }
}

#endif

void handle_input() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP || e.type == SDL_MOUSEMOTION || e.type ==
            SDL_MOUSEMOTION || e.type == SDL_MOUSEWHEEL) {
            scale_mouse_event(&e);
        }
        if (input_event_count < 128) {
            input_events[input_event_count] = e;
            input_event_count++;
        }

        switch (e.type) {
            case SDL_WINDOWEVENT:
#ifdef _WIN32
                if (!g_config.video.windowed) {
                    if (e.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
                        MakeWindowTopMost(renderer.window);
                    } else if (e.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
                        MakeWindowNormal(renderer.window);
                    }
                }
#endif
                if (g_config.video.windowed) {
                    if (e.window.event == SDL_WINDOWEVENT_RESIZED) {
                        if (!window_resetting) {
                            char keybind_string[32];
                            BindingDescriptor *reset_window_binding = binding_find_by_id(RESET_WINDOW_SIZE_HOTKEY);;
                            if (reset_window_binding) {
                                xlog(2, "Game window resized, %s to reset it.",
                                     keybinding_to_string(
                                         reset_window_binding->keybinding,
                                         keybind_string, 32));
                            }
                        }
                        if ((SDL_GetWindowFlags(renderer.window) & SDL_WINDOW_MAXIMIZED) != 0) {
                            g_config.video.window_size[0] = e.window.data1;
                            g_config.video.window_size[1] = e.window.data2;
                            resize_fbo_scaling(g_config.video.window_size[0], g_config.video.window_size[1]);
                            continue; // If they are maximizing the window just set the new size for scaling
                        }

                        // Otherwise attempt to maintain aspect ratio.
                        int difference_width = abs(g_config.video.window_size[0] - e.window.data1);
                        int difference_height = abs(g_config.video.window_size[1] - e.window.data2);
                        bool widthStretch = difference_width > difference_height;
                        if (widthStretch) {
                            int differenceFromNatural = e.window.data1 - SCREEN_WIDTH;
                            g_config.video.window_size[0] = e.window.data1;
                            g_config.video.window_size[1] =
                                    SCREEN_HEIGHT + (9.0f / 16.0f * (float) differenceFromNatural);
                        } else {
                            int differenceFromNatural = e.window.data2 - SCREEN_HEIGHT;
                            g_config.video.window_size[0] =
                                    SCREEN_WIDTH + (16.0f / 9.0f * (float) differenceFromNatural);
                            g_config.video.window_size[1] = e.window.data2;
                        }
                        SDL_SetWindowSize(renderer.window, g_config.video.window_size[0],
                                          g_config.video.window_size[1]);
                        resize_fbo_scaling(g_config.video.window_size[0], g_config.video.window_size[1]);
                        window_resetting = false;
                    }
                }
                break;
            case SDL_QUIT: quit = 1;
                break;
            case SDL_KEYDOWN:
            case SDL_TEXTINPUT:
            case SDL_MOUSEMOTION:
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
            case SDL_MOUSEWHEEL:
            default:
                if (g_game_state == GAME_STATE_LAUNCHER) {
                    launcher_handle_input(&e);
                } else if (g_game_state == GAME_STATE_GAME) {
                    game_handle_input(&e);
                }
                break;
        }
        if (e.type == SDL_QUIT) quit = 1;
    }
}
