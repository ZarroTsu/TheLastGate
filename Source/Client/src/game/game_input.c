#include "game_input.h"

#include <stdbool.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_syswm.h>

#include "game_ui.h"
#include "../../inter.h"
#include "../../main.h"
#include "../net/socket.h"
#include "../../engine.h"
#include "config/config.h"
#include "graphics/atlas.h"
#include "graphics/sdl.h"
#include "config/keybindings.h"
#include "input/input.h"
#include "ui/imgui/imgui_wrapper.h"

bool waiting_for_keybind = false;

static ScrollableRegion get_scrollable_region(int x, int y) {
    if (x > gui_inv_x[0] && x < gui_inv_x[1] && y > gui_inv_y[0] && y < gui_inv_y[1])
        return INVENTORY;

    if (x > gui_skl_names[RECT_X1] && x < gui_skl_names[RECT_X2] + 110 && y > gui_skl_names[RECT_Y1] && y <
        gui_skl_names[RECT_Y2])
        return SKILL_LIST;

    if (game_ui_state.show_waypoints && x > (((1280 / 2) - (320 / 2))) && x < (((1280 / 2) - (320 / 2)) + 280 - 13) && y > (
            ((736 / 2) - (320 / 2) + 72) + 1) && y < (((736 / 2) - (320 / 2) + 72) + 1 + 280))
        return WAYPOINT_PAGE;

    if (game_ui_state.open_shop == 112 && x > (((1280 / 2) - (320 / 2))) && x < (((1280 / 2) - (320 / 2)) + 280 - 13) && y > (
            ((736 / 2) - (320 / 2) + 72) + 1) && y < (((736 / 2) - (320 / 2) + 72) + 1 + 280))
        return DEPOT_PAGE;

    if (game_ui_state.show_options && x > (((1280 / 2) - (320 / 2))) && x < (((1280 / 2) - (320 / 2)) + 280 - 13) && y > (
            ((736 / 2) - (320 / 2) + 72) + 1) && y < (((736 / 2) - (320 / 2) + 72) + 1 + 280)) {
        return OPTIONS_WINDOW;
    }

    if (x > 973 && x < 1275 && y > 6 && y < 230)
        return CHAT_HISTORY;


    return NONE;
}

void game_input_init(void) {
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
        game_ui_state.show_options = !game_ui_state.show_options;
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
    if (strcmp(id, MY_BUFF_HOTKEY) == 0) {
        say("/buffs");
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
    if (strcmp(id, GC_TRANSFER_HOTKEY) == 0) {
        say("transfer");
        return;
    }
}


void game_handle_input(const SDL_Event *e) {
    switch (e->type) {
        case SDL_KEYDOWN:
            if (waiting_for_keybind) break;

            BindingDescriptor *binding = binding_find(e->key.keysym.sym, SDL_GetModState());

            if (binding && !(g_config.ui.enter_to_talk && chat_mode_active)) {
                handle_hotkey(binding);
            }

            switch (e->key.keysym.sym) {
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
                    if (game_ui_state.open_shop != 0) {
                        game_ui_state.open_shop = 0;
                        noshop = QSIZE * 3;
                        closed_window = true;
                    }
                    if (game_ui_state.show_waypoints ) {
                        game_ui_state.show_waypoints = false;
                        closed_window = true;
                    }
                    if (game_ui_state.open_skill_tree != 0) {
                        game_ui_state.open_skill_tree = 0;
                        closed_window = true;
                    }
                    if (game_ui_state.open_book != 0) {
                        game_ui_state.open_book = 0;
                        closed_window = true;
                    }
                    if (game_ui_state.show_motd) {
                        game_ui_state.show_motd = false;
                        closed_window = true;
                    }
                    if (game_ui_state.show_new_player) {
                        game_ui_state.show_new_player = false;
                        closed_window = true;
                    }
                    if (game_ui_state.tutorial.open != 0) {
                        game_ui_state.tutorial.open = 0;
                        closed_window = true;
                    }
                    if (game_ui_state.show_options) {
                        game_ui_state.show_options = false;
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
                    if (!isalnum(e->text.text[0])) in_len--;
                    cur_pos = in_len;
                    tabmode = 0;
                    tabstart = 0;
                }
                if (cur_pos > in_len) cur_pos = in_len;
                memmove(input + cur_pos + 1, input + cur_pos, 120 - cur_pos);
                input[cur_pos] = e->text.text[0];
                in_len++;
                cur_pos++;
            }
            break;
        case SDL_MOUSEMOTION:
            mouse(e->motion.x, e->motion.y, MS_MOVE);
            mx = e->motion.x;
            my = e->motion.y;
            break;
        case SDL_MOUSEBUTTONDOWN:
            if (is_mouse_over_imgui()) break;
            mouse(e->button.x, e->button.y, e->button.button == SDL_BUTTON_LEFT ? MS_LB_DOWN : MS_RB_DOWN);
            break;
        case SDL_MOUSEBUTTONUP:
            if (is_mouse_over_imgui()) break;
            mouse(e->button.x, e->button.y, e->button.button == SDL_BUTTON_LEFT ? MS_LB_UP : MS_RB_UP);
            break;
        case SDL_MOUSEWHEEL:
            if (is_mouse_over_imgui()) break;
            const int delta = e->wheel.y;
            const int x = e->wheel.mouseX;
            const int y = e->wheel.mouseY;
            switch (get_scrollable_region(x, y)) {
                case INVENTORY:
                    if (delta < 0 && game_ui_state.inventory_scroll < MAXITEMS - 30)
                        game_ui_state.inventory_scroll += 10;
                    else if (delta > 0 && game_ui_state.inventory_scroll > 1)
                        game_ui_state.inventory_scroll -= 10;
                    break;
                case SKILL_LIST:
                    if (game_ui_state.hud_mode == HUD_MODE_LIST_OFFENSES || game_ui_state.hud_mode == HUD_MODE_LIST_DEFENSES) {
                        if (delta < 0 && game_ui_state.meta_scroll < MAX_META_SCROLL)
                            game_ui_state.meta_scroll++;
                        else if (delta > 0 && game_ui_state.meta_scroll > 0)
                            game_ui_state.meta_scroll--;
                    } else {
                        if (delta < 0 && game_ui_state.skill_scroll < MAX_SKILL_SCROLL)
                            game_ui_state.skill_scroll++;
                        else if (delta > 0 && game_ui_state.skill_scroll > 0)
                            game_ui_state.skill_scroll--;
                    }
                    break;
                case WAYPOINT_PAGE:
                    if (delta < 0 && game_ui_state.waypoint_scroll < MAXWPS - 8)
                        game_ui_state.waypoint_scroll++;
                    else if (delta > 0 && game_ui_state.waypoint_scroll > 0)
                        game_ui_state.waypoint_scroll--;
                    break;
                case DEPOT_PAGE:
                    if (delta < 0 && game_ui_state.open_depot_page < 7)
                        game_ui_state.open_depot_page++;
                    else if (delta > 0 && game_ui_state.open_depot_page > 0)
                        game_ui_state.open_depot_page--;
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
}
