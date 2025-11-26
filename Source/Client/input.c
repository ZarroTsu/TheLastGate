#include "input.h"

#include <SDL2/SDL_events.h>

#include "inter.h"
#include "main.h"
#include "socket.h"
#include "engine.h"
#include "graphics/sdl.h"

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

    if (x > 973 && x < 1275 && y > 6 && y < 230)
        return CHAT_HISTORY;


    return NONE;
}

void init_input(void) {
    SDL_StartTextInput();
}

void scale_mouse_position(int *x, int *y) {
    if (!app_state.windowed) {
        float scaleX = 1280 / (float)app_state.window_size[0];
        float scaleY = 720 / (float)app_state.window_size[1];
        *x *= scaleX;
        *y *= scaleY;
    }
}

#ifdef _WIN32

#include <SDL2/SDL_syswm.h>

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

void handle_input(void) {
    SDL_Event e;

    while (SDL_PollEvent(&e)) {
        bool spellKey = (SDL_GetModState() & (KMOD_CTRL | KMOD_ALT)) != 0;
        switch (e.type) {
            case SDL_WINDOWEVENT:
#ifdef _WIN32
                if (e.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
                    MakeWindowTopMost(app.window);
                } else if (e.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
                    MakeWindowNormal(app.window);
                }
#endif
                break;
            case SDL_QUIT: quit = 1;
                break;
            case SDL_KEYDOWN:
                switch (e.key.keysym.sym) {
                    // Spells 1-5
                    case SDLK_1:
                        if (spellKey) button_command(16);
                        break;
                    case SDLK_2:
                        if (spellKey) (button_command(17));
                        break;
                    case SDLK_3:
                        if (spellKey) button_command(18);
                        break;
                    case SDLK_4:
                        if (spellKey) button_command(19);
                        break;
                    case SDLK_5:
                        if (spellKey) button_command(20);
                        break;

                    // Spells Q-T
                    case SDLK_q:
                        if (spellKey) button_command(21);
                        break;
                    case SDLK_w:
                        if (spellKey) button_command(22);
                        break;
                    case SDLK_e:
                        if (spellKey) button_command(23);
                        break;
                    case SDLK_r:
                        if (spellKey) button_command(24);
                        break;
                    case SDLK_t:
                        if (spellKey) button_command(25);
                        break;

                    // Spells A-G
                    case SDLK_a:
                        if (spellKey) button_command(26);
                        break;
                    case SDLK_s:
                        if (spellKey) button_command(27);
                        break;
                    case SDLK_d:
                        if (spellKey) button_command(28);
                        break;
                    case SDLK_f:
                        if (spellKey) button_command(29);
                        break;
                    case SDLK_g:
                        if (spellKey) button_command(30);
                        break;

                    // Spells Z-V
                    case SDLK_z:
                        if (spellKey) button_command(31);
                        break;
                    case SDLK_x:
                        if (spellKey) button_command(32);
                        break;
                    case SDLK_c:
                        if (spellKey) button_command(33);
                        break;
                    case SDLK_v:
                        if (spellKey) button_command(34);
                        break;
                    case SDLK_b:
                        if (spellKey) button_command(35);
                        break;

                    case SDLK_ESCAPE:
                        cmd(CL_CMD_RESET, 0, 0);
                        show_shop = 0;
                        show_wps = 0;
                        show_tree = 0;
                        show_book = 0;
                        show_motd = 0;
                        show_newp = 0;
                        show_tuto = 0;
                        noshop = QSIZE * 3;
                        break;

                    case SDLK_F1:
                        cmd(CL_CMD_MODE, 2, 0);
                        break;
                    case SDLK_F2:
                        cmd(CL_CMD_MODE, 1, 0);
                        break;
                    case SDLK_F3:
                        cmd(CL_CMD_MODE, 0, 0);
                        break;
                    case SDLK_F4:
                        pdata.show_proz = 1 - pdata.show_proz;
                        break;

                    case SDLK_F5:
                        pdata.show_stats = 1 - pdata.show_stats;
                        break;
                    case SDLK_F6:
                        pdata.hide = 1 - pdata.hide;
                        break;
                    case SDLK_F7:
                        pdata.show_names = 1 - pdata.show_names;
                        break;
                    case SDLK_F8:
                        pdata.show_bars = 1 - pdata.show_bars;
                        break;

                    case SDLK_F9:
                        // dd_savescreen(); TODO: Implement this
                        break;
                    case SDLK_F10:
                        app_state.gamma += 250;
                        if (app_state.gamma > 6000) app_state.gamma = 5000;
                        xlog(2, "Set gamma correction to %1.2f", app_state.gamma / 5000.0);
                        break;
                    case SDLK_F11:
                        xlog(2, " ");
                        xlog(2, "Client Version %d.%02d.%02d",VERSION >> 16, (VERSION >> 8) & 255,VERSION & 255);
                        xlog(2, "Server Version %d.%02d.%02d", ser_ver >> 16, (ser_ver >> 8) & 255, ser_ver & 255);
                        xlog(2, "R=%04X, G=%04X, B=%04X", RED, GREEN, BLUE);
                        xlog(2, "Skip=%d%% Idle=%d%%", pskip, pidle);
                        break;
                    case SDLK_F12:
                        cmd_exit();
                        break;
                    case SDLK_INSERT:
                        cmd3(CL_CMD_INV, 9, 1, selected_char);
                        break;

                    case SDLK_TAB:
                        complete_word();
                        break;

                    case SDLK_BACKSPACE:
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
                        cur_pos = 0;
                        tabmode = 0;
                        tabstart = 0;
                        break;
                    case SDLK_END:
                        cur_pos = in_len;
                        tabmode = 0;
                        tabstart = 0;
                        break;
                    case SDLK_LEFT:
                        if (cur_pos) cur_pos--;
                        tabmode = 0;
                        tabstart = 0;
                        break;
                    case SDLK_RIGHT:
                        if (cur_pos < 115) cur_pos++;
                        tabmode = 0;
                        tabstart = 0;
                        break;
                    case SDLK_UP:
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
                    case SDLK_RETURN:
                    case SDLK_KP_ENTER:
                        printf("Enter pressed\n");
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

                        break;
                    default:
                        break;
                }
                break; // End of SDL_KEYDOWN

            case SDL_TEXTINPUT:
                if (in_len < 115 && !spellKey) {
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
                if (!app_state.windowed) scale_mouse_position(&e.button.x, &e.button.y);
                mouse(e.button.x, e.button.y, e.button.button == SDL_BUTTON_LEFT ? MS_LB_DOWN : MS_RB_DOWN);
                break;
            case SDL_MOUSEBUTTONUP:
                if (!app_state.windowed) scale_mouse_position(&e.button.x, &e.button.y);
                mouse(e.button.x, e.button.y, e.button.button == SDL_BUTTON_LEFT ? MS_LB_UP : MS_RB_UP);
                break;
            case SDL_MOUSEWHEEL:
                if (!app_state.windowed) scale_mouse_position(&e.wheel.mouseX, &e.wheel.mouseY);
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
