#include "input.h"

#include <stdbool.h>
#include <stdio.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_syswm.h>

#include "inter.h"
#include "main.h"
#include "src/net/socket.h"
#include "engine.h"
#include "config/config.h"
#include "graphics/atlas.h"
#include "graphics/scaling.h"
#include "graphics/sdl.h"
#include "ui/option_window.h"
#include "config/keybindings.h"
#include "ui/imgui/imgui_wrapper.h"

bool waiting_for_keybind = false;
static bool window_resetting = false;

static ScrollableRegion get_scrollable_region(int x, int y) {
    if (x > gui_inv_x[0] && x < gui_inv_x[1] && y > gui_inv_y[0] && y < gui_inv_y[1])
        return INVENTORY;

    if (x > gui_skl_names[RECT_X1] && x < gui_skl_names[RECT_X2] + 110 && y > gui_skl_names[RECT_Y1] && y <
        gui_skl_names[RECT_Y2])
        return SKILL_LIST;

    if (show_wps && x > (((1280 / 2) - (320 / 2))) && x < (((1280 / 2) - (320 / 2)) + 280 - 13) && y > (
            ((736 / 2) - (320 / 2) + 72) + 1) && y < (((736 / 2) - (320 / 2) + 72) + 1 + 280))
        return WAYPOINT_PAGE;

    if (show_shop == 112 && x > (((1280 / 2) - (320 / 2))) && x < (((1280 / 2) - (320 / 2)) + 280 - 13) && y > (
            ((736 / 2) - (320 / 2) + 72) + 1) && y < (((736 / 2) - (320 / 2) + 72) + 1 + 280))
        return DEPOT_PAGE;

    if (show_opts && x > (((1280 / 2) - (320 / 2))) && x < (((1280 / 2) - (320 / 2)) + 280 - 13) && y > (
            ((736 / 2) - (320 / 2) + 72) + 1) && y < (((736 / 2) - (320 / 2) + 72) + 1 + 280)) {
        return OPTIONS_WINDOW;
    }

    if (x > 973 && x < 1275 && y > 6 && y < 230)
        return CHAT_HISTORY;


    return NONE;
}

void init_input(void) {
    if (!g_config.ui.enter_to_talk) {
        SDL_StartTextInput();
    } else {
        chat_mode_active = false;
        SDL_StopTextInput();
    }
}

void sync_chat_input_state(void) {
    if (g_config.ui.enter_to_talk) {
        if (chat_mode_active) {
            chat_mode_active = 0;
        }
        SDL_StopTextInput();
    } else {
        chat_mode_active = 0;
        SDL_StartTextInput();
    }
}

static int is_mouse_over_imgui(void) {
    return imgui_want_capture_mouse();
}

void scale_mouse_event(SDL_Event *event) {
    /* Scale mouse coordinates from window space to virtual 1280x720 space for ImGui */
    if (g_config.video.window_size[0] == SCREEN_WIDTH && g_config.video.window_size[1] == SCREEN_HEIGHT) {
        return; /* No scaling needed */
    }

    float scaleX = (float)SCREEN_WIDTH / (float)g_config.video.window_size[0];
    float scaleY = (float)SCREEN_HEIGHT / (float)g_config.video.window_size[1];

    switch (event->type) {
        case SDL_MOUSEMOTION:
            event->motion.x = (int)(event->motion.x * scaleX);
            event->motion.y = (int)(event->motion.y * scaleY);
            break;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            event->button.x = (int)(event->button.x * scaleX);
            event->button.y = (int)(event->button.y * scaleY);
            break;
        case SDL_MOUSEWHEEL:
            event->wheel.mouseX = (int)(event->wheel.mouseX * scaleX);
            event->wheel.mouseY = (int)(event->wheel.mouseY * scaleY);
            break;
    }
}

static void handle_hotkey(const BindingDescriptor *binding) {
    const char *id = binding->id;

    if (strncmp(id, "spell_", 6) == 0) {
        int spell_num = atoi(id + 6);
        button_command(16 + spell_num - 1);
        return;
    }

    /* Speed Hotkeys */
    if (strcmp(id, SPEED_FAST_HOTKEY) == 0) {
        cmd(CL_CMD_MODE, 2, 0);
        return;
    }
    if (strcmp(id, SPEED_NORMAL_HOTKEY) == 0) {
        cmd(CL_CMD_MODE, 1, 0);
        return;
    }
    if (strcmp(id, SPEED_SLOW_HOTKEY) == 0) {
        cmd(CL_CMD_MODE, 0, 0);
        return;
    }

    /* World Hotkeys */
    if (strcmp(id, TOGGLE_PERCENT_HOTKEY) == 0) {
        pdata.show_proz = 1 - pdata.show_proz;
        return;
    }
    if (strcmp(id, TOGGLE_STAT_BASE_HOTKEY) == 0) {
        pdata.show_stats = 1 - pdata.show_stats;
        return;
    }
    if (strcmp(id, TOGGLE_HIDE_SPRITE_HOTKEY) == 0) {
        pdata.hide = 1 - pdata.hide;
        return;
    }
    if (strcmp(id, TOGGLE_NAMES_HOTKEY) == 0) {
        pdata.show_names = 1 - pdata.show_names;
        return;
    }
    if (strcmp(id, TOGGLE_HEALTH_BARS_HOTKEY) == 0) {
        pdata.show_bars = 1 - pdata.show_bars;
        return;
    }

    /* Window Hotkeys */
    if (strcmp(id, TOGGLE_OPTIONS_HOTKEY) == 0) {
        // dd_savescreen(); TODO: Implement this
        show_opts = !show_opts;
        apply_config_changes();
        return;
    }
    if (strcmp(id, TOGGLE_GAMMA_HOTKEY) == 0) {
        g_config.video.gamma += 250;
        if (g_config.video.gamma > 6000) g_config.video.gamma = 5000;
        xlog(2, "Set gamma correction to %1.2f", g_config.video.gamma / 5000.0);
        return;
    }
    if (strcmp(id, RESET_WINDOW_SIZE_HOTKEY) == 0) {
        if (g_config.video.windowed) {
            g_config.video.window_size[0] = SCREEN_WIDTH;
            g_config.video.window_size[1] = SCREEN_HEIGHT;
            SDL_RestoreWindow(renderer.window);
            window_resetting = true;
            SDL_SetWindowSize(renderer.window, g_config.video.window_size[0], g_config.video.window_size[1]);
            xlog(2, "Game window reset to default resolution.");
        }
        return;
    }

    /* General Hotkeys */
    if (strcmp(id, FIGHTBACK_HOTKEY) == 0) {
        say("/fightback");
        return;
    }
    if (strcmp(id, SWAP_POSITION_HOTKEY) == 0) {
        say("/swap");
        return;
    }
    if (strcmp(id, SWAP_GEAR_HOTKEY) == 0) {
        cmd3(CL_CMD_INV, 9, 1, selected_char);
        return;
    }
    if (strcmp(id, WHO_HOTKEY) == 0) {
        say("/who");
        return;
    }
    if (strcmp(id, DISPLAY_DEBUG_INFORMATION_HOTKEY) == 0) {
        SDL_SysWMinfo info;
        SDL_VERSION(&info.version);
        const GLubyte *version = glGetString(GL_VERSION);
        const GLubyte *vendor = glGetString(GL_VENDOR);
        const GLubyte *gl_renderer = glGetString(GL_RENDERER);
        const GLubyte *glsl = glGetString(GL_SHADING_LANGUAGE_VERSION);

        xlog(2, " ");
        xlog(2, "Client Version RC%d.%02d", CLIENT_VERSION.major, CLIENT_VERSION.minor);
        xlog(2, "Networking Version %d.%02d.%02d",NETWORKING_VERSION >> 16, (NETWORKING_VERSION >> 8) & 255,
             NETWORKING_VERSION & 255);
        xlog(2, "Server Version %d.%02d.%02d", ser_ver >> 16, (ser_ver >> 8) & 255, ser_ver & 255);
        xlog(2, "SDL Version %d.%02d.%02d", info.version.major, info.version.minor, info.version.patch);
        xlog(2, "OpenGL Version: %s", version);
        xlog(2, "OpenGL Vendor: %s", vendor);
        xlog(2, "OpenGL Renderer: %s", gl_renderer);
        xlog(2, "GLSL Version: %s", glsl);
        xlog(2, "R=%04X, G=%04X, B=%04X", RED, GREEN, BLUE);
        xlog(2, "Skip=%d%% Idle=%d%%", pskip, pidle);
        log_atlas_debug_info();
        return;
    }
    if (strcmp(id, EXIT_HOTKEY) == 0) {
        cmd_exit();
        return;
    }

    /* GC Hotkeys */
    if (strcmp(id, GC_OFFENSE_HOTKEY) == 0) {
        say("Offense");
        return;
    }

    if (strcmp(id, GC_DEFENSE_HOTKEY) == 0) {
        say("Defense");
        return;
    }
    if (strcmp(id, GC_PASSIVE_HOTKEY) == 0) {
        say("Passive");
        return;
    }
    if (strcmp(id, GC_WAIT_HOTKEY) == 0) {
        say("Wait");
        return;
    }
    if (strcmp(id, GC_FOLLOW_HOTKEY) == 0) {
        say("Follow");
        return;
    }
    if (strcmp(id, GC_MOVE_HOTKEY) == 0) {
        say("Move");
        return;
    }
    if (strcmp(id, GC_BUFFS_HOTKEY) == 0) {
        say("/gcb");
        return;
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


SDL_Event input_events[128];
int input_event_count = 0;

#endif

void handle_input(void) {
    SDL_Event e;


    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP || e.type == SDL_MOUSEMOTION || e.type == SDL_MOUSEMOTION || e.type == SDL_MOUSEWHEEL) {
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
                            g_config.video.window_size[1] = SCREEN_HEIGHT + (9.0f / 16.0f * (float)differenceFromNatural);
                        } else {
                            int differenceFromNatural = e.window.data2 - SCREEN_HEIGHT;
                            g_config.video.window_size[0] = SCREEN_WIDTH + ( 16.0f / 9.0f * (float)differenceFromNatural);
                            g_config.video.window_size[1] = e.window.data2;
                        }
                        SDL_SetWindowSize(renderer.window, g_config.video.window_size[0], g_config.video.window_size[1]);
                        resize_fbo_scaling(g_config.video.window_size[0], g_config.video.window_size[1]);
                        window_resetting = false;
                    }
                }
                break;
            case SDL_QUIT: quit = 1;
                break;
            case SDL_KEYDOWN:
                if (waiting_for_keybind) break;

                BindingDescriptor *binding = binding_find(e.key.keysym.sym, SDL_GetModState());

                if (binding && !(g_config.ui.enter_to_talk && chat_mode_active)) {
                    handle_hotkey(binding);
                }

                switch (e.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        if (g_config.ui.enter_to_talk && chat_mode_active) {
                            /* Clear input buffer */
                            input[0] = 0;
                            in_len = 0;
                            cur_pos = 0;
                            view_pos = 0;
                            hist_nr = 0;
                            /* Reset tab completion state */
                            tabmode = 0;
                            tabstart = 0;

                            chat_mode_active = 0;
                            SDL_StopTextInput();
                            break;
                        }

                        bool closed_window = false;
                        if (show_shop != 0) {
                            show_shop = 0;
                            noshop = QSIZE * 3;
                            closed_window = true;
                        }
                        if (show_wps != 0) {
                            show_wps = 0;
                            closed_window = true;
                        }
                        if (show_tree != 0) {
                            show_tree = 0;
                            closed_window = true;
                        }
                        if (show_book != 0) {
                            show_book = 0;
                            closed_window = true;
                        }
                        if (show_motd != 0) {
                            show_motd = 0;
                            closed_window = true;
                        }
                        if (show_newp != 0) {
                            show_newp = 0;
                            closed_window = true;
                        }
                        if (show_tuto != 0) {
                            show_tuto = 0;
                            closed_window = true;
                        }
                        if (show_opts != 0) {
                            show_opts = 0;
                            apply_config_changes();
                            closed_window = true;
                        }
                        if (!g_config.ui.escape_closes_menu_first || !closed_window) cmd(CL_CMD_RESET, 0, 0);
                        break;
                    case SDLK_TAB:
                        if (g_config.ui.enter_to_talk && !chat_mode_active) {
                            break;
                        }
                        complete_word();
                        break;

                    case SDLK_BACKSPACE:
                        if (g_config.ui.enter_to_talk && !chat_mode_active) {
                            break;
                        }
                        if (cur_pos && in_len) {
                            if (tabmode) {
                                in_len = cur_pos;
                                tabmode = 0;
                                tabstart = 0;
                            }
                            if (cur_pos > in_len) cur_pos = in_len;
                            memmove(input + cur_pos - 1, input + cur_pos, 120 - cur_pos);
                            in_len--;
                            cur_pos--;
                        }
                        break;

                    case SDLK_DELETE:
                        if (g_config.ui.enter_to_talk && !chat_mode_active) {
                            break;
                        }
                        if (in_len) {
                            if (tabmode) {
                                in_len = cur_pos;
                                tabmode = 0;
                                tabstart = 0;
                            } else {
                                memmove(input + cur_pos, input + cur_pos + 1, 120 - cur_pos);
                                in_len--;
                            }
                        }
                        break;
                    case SDLK_PAGEUP:
                        if (logstart < 22 * 8) {
                            logstart += 11;
                            if (logstart > 22 * 8) logstart = 22 * 8;
                            logtimer = TICKS * 30 / TICKMULTI;
                        }
                        break;
                    case SDLK_PAGEDOWN:
                        if (logstart > 0) {
                            logstart -= 11;
                            if (logstart < 0) logstart = 0;
                            logtimer = TICKS * 30 / TICKMULTI;
                        }
                        break;
                    case SDLK_HOME:
                        if (g_config.ui.enter_to_talk && !chat_mode_active) {
                            break;
                        }
                        cur_pos = 0;
                        tabmode = 0;
                        tabstart = 0;
                        break;
                    case SDLK_END:
                        if (g_config.ui.enter_to_talk && !chat_mode_active) {
                            break;
                        }
                        cur_pos = in_len;
                        tabmode = 0;
                        tabstart = 0;
                        break;
                    case SDLK_LEFT:
                        if (g_config.ui.enter_to_talk && !chat_mode_active) {
                            break;
                        }
                        if (cur_pos) cur_pos--;
                        tabmode = 0;
                        tabstart = 0;
                        break;
                    case SDLK_RIGHT:
                        if (g_config.ui.enter_to_talk && !chat_mode_active) {
                            break;
                        }
                        if (cur_pos < 115) cur_pos++;
                        tabmode = 0;
                        tabstart = 0;
                        break;
                    case SDLK_UP:
                        if (g_config.ui.enter_to_talk && !chat_mode_active) {
                            break;
                        }
                        if (hist_nr < 19) {
                            memcpy(history[hist_nr], input, 128);
                            hist_len[hist_nr] = in_len;
                            hist_nr++;

                            memcpy(input, history[hist_nr], 128);
                            in_len = cur_pos = hist_len[hist_nr];

                            tabmode = 0;
                            tabstart = 0;
                        }
                        break;
                    case SDLK_DOWN:
                        if (g_config.ui.enter_to_talk && !chat_mode_active) {
                            break;
                        }
                        if (hist_nr > 0) {
                            memcpy(history[hist_nr], input, 128);
                            hist_len[hist_nr] = in_len;

                            hist_nr--;

                            memcpy(input, history[hist_nr], 128);
                            in_len = cur_pos = hist_len[hist_nr];

                            tabmode = 0;
                            tabstart = 0;
                        }
                        break;
                    case SDLK_SLASH:
                        if (g_config.ui.enter_to_talk && !chat_mode_active) {
                            SDL_StartTextInput();
                            chat_mode_active = 1;
                            memmove(input + cur_pos + 1, input + cur_pos, 120 - cur_pos);
                            input[cur_pos] = '/';
                            in_len++;
                            cur_pos++;
                        }
                        break;
                    case SDLK_RETURN:
                    case SDLK_KP_ENTER:
                        if (g_config.ui.enter_to_talk) {
                            if (!chat_mode_active) {
                                chat_mode_active = 1;
                                SDL_StartTextInput();
                                break;
                            }

                            if (in_len > 0) {
                                if (tabmode) {
                                    tabmode = 0;
                                    tabstart = 0;
                                    in_len--;
                                }

                                memmove(history[2], history[1], 18 * 128);
                                memmove(&hist_len[2], &hist_len[1], sizeof(int) * 18);

                                memcpy(history[1], input, 128);
                                hist_len[1] = in_len;

                                input[in_len] = 0;
                                in_len = 0;
                                cur_pos = 0;
                                view_pos = 0;
                                hist_nr = 0;

                                add_words();

                                say(input);
                            }

                            chat_mode_active = 0;
                            SDL_StopTextInput();
                            tabmode = 0;
                            tabstart = 0;
                        } else {
                            if (in_len == 0) break;

                            if (tabmode) {
                                tabmode = 0;
                                tabstart = 0;
                                in_len--;
                            }

                            memmove(history[2], history[1], 18 * 128);
                            memmove(&hist_len[2], &hist_len[1], sizeof(int) * 18);

                            memcpy(history[1], input, 128);
                            hist_len[1] = in_len;

                            input[in_len] = 0;
                            in_len = 0;
                            cur_pos = 0;
                            view_pos = 0;
                            hist_nr = 0;

                            add_words();

                            say(input);
                        }
                        break;
                    default:
                        break;
                }
                break; // End of SDL_KEYDOWN

            case SDL_TEXTINPUT:
                if (g_config.ui.enter_to_talk && !chat_mode_active) {
                    break;
                }

                int mods = SDL_GetModState();
                bool has_spell_modifier = ((mods & KMOD_CTRL) != 0) || ((mods & KMOD_ALT) != 0);
                if (in_len < 115 && !has_spell_modifier) {
                    if (tabmode) {
                        if (!isalnum(e.text.text[0])) in_len--;
                        cur_pos = in_len;
                        tabmode = 0;
                        tabstart = 0;
                    }
                    if (cur_pos > in_len) cur_pos = in_len;
                    memmove(input + cur_pos + 1, input + cur_pos, 120 - cur_pos);
                    input[cur_pos] = e.text.text[0];
                    in_len++;
                    cur_pos++;
                }
                break;
            case SDL_MOUSEMOTION:
                mouse(e.motion.x, e.motion.y, MS_MOVE);
                mx = e.motion.x;
                my = e.motion.y;
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (is_mouse_over_imgui()) break;
                mouse(e.button.x, e.button.y, e.button.button == SDL_BUTTON_LEFT ? MS_LB_DOWN : MS_RB_DOWN);
                break;
            case SDL_MOUSEBUTTONUP:
                if (is_mouse_over_imgui()) break;
                mouse(e.button.x, e.button.y, e.button.button == SDL_BUTTON_LEFT ? MS_LB_UP : MS_RB_UP);
                break;
            case SDL_MOUSEWHEEL:
                if (is_mouse_over_imgui()) break;
                const int delta = e.wheel.y;
                const int x = e.wheel.mouseX;
                const int y = e.wheel.mouseY;
                switch (get_scrollable_region(x, y)) {
                    case INVENTORY:
                        if (delta < 0 && inv_pos < MAXITEMS - 30)
                            inv_pos += 10;
                        else if (delta > 0 && inv_pos > 1)
                            inv_pos -= 10;
                        break;
                    case SKILL_LIST:
                        if (delta < 0 && skill_pos < MAXSKILL - 10)
                            skill_pos++;
                        else if (delta > 0 && skill_pos > 0)
                            skill_pos--;
                        break;
                    case WAYPOINT_PAGE:
                        if (delta < 0 && wps_pos < MAXWPS - 8)
                            wps_pos++;
                        else if (delta > 0 && wps_pos > 0)
                            wps_pos--;
                        break;
                    case DEPOT_PAGE:
                        if (delta < 0 && dept_page < 7)
                            dept_page++;
                        else if (delta > 0 && dept_page > 0)
                            dept_page--;
                        break;
                    case CHAT_HISTORY:
                        if (delta < 0 && logstart > 0) {
                            logstart -= 3;
                            logtimer = TICKS * 30 / TICKMULTI;
                        } else if (delta > 0 && logstart < 22 * 8) {
                            logstart += 3;
                            logtimer = TICKS * 30 / TICKMULTI;
                        }
                        break;
                    default: break;
                }
        }
        if (e.type == SDL_QUIT) quit = 1;
    }
}
