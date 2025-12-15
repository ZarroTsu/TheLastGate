//
// Created by james on 12/13/2025.
//

#include "stubborn_actions.h"

#include <stdbool.h>

#include "inter.h"
#include "main.h"
#include "use_queue.h"
#include "../../../Server/macros.h"
#include "config/config.h"

static StubbornActionState state = {0};

static int center() {
    return (screen_renderdist / 2) + (screen_renderdist / 2) * screen_renderdist;
}

static int north() {
    // 176
    return center() - 1;
}

static int east() {
    // 168
    return center() + 1 * screen_renderdist;
}

static int south() {
    // 184
    return center() + 1;
}

static int west() {
    // 160
    return center() - 1 * screen_renderdist;
}

static bool is_player_using(int offset) {
    struct cmap player = map[center()];
    if (player.ch_stat_off != 4) return false;
    if (player.ch_status == 160 + offset && map[west()].ch_id == 0) return true;
    if (player.ch_status == 168 + offset && map[east()].ch_id == 0) return true;
    if (player.ch_status == 176 + offset && map[north()].ch_id == 0) return true;
    if (player.ch_status == 184 + offset && map[south()].ch_id == 0) return true;
    return false;
}

static Uint32 get_next_attempt_time() {
    return max(1000, min(5000, 1000 * state.number_of_attempts));
}

static bool is_stubborn_action_enabled(int action) {
    switch (action) {
        case CL_CMD_USE:
            if (g_config.gameplay.stubborn_use) return true;
            break;
        case CL_CMD_DROP:
            if (g_config.gameplay.stubborn_drop) return true;
            break;
        case CL_CMD_PICKUP:
            if (g_config.gameplay.stubborn_pickup) return true;
            break;
        case CL_CMD_MOVE:
            if (g_config.gameplay.stubborn_move) return true;
            break;
        case CL_CMD_GIVE:
            if (g_config.gameplay.stubborn_give) return true;
            break;
        default:
            return false;
    }
}

void mod_stubborn_actions_set_use_cmd(int x, int y) {
    if (!g_config.gameplay.stubborn_use) return;
    state.pending_action = CL_CMD_USE;
    state.target_x = x;
    state.target_y = y;
    state.last_x = map[center()].x;
    state.last_y = map[center()].y;
    state.number_of_attempts = 0;
    state.next_attempt = get_next_attempt_time();
    state.indicator = false;
    state.last_combat_target = 0;
}

void mod_stubborn_actions_on_cmd(const int action, const int tile_id) {
    if (!is_stubborn_action_enabled(action)) return;
    state.pending_action = action;
    state.target_x = map[tile_id].x;
    state.target_y = map[tile_id].y;
    state.last_x = map[center()].x;
    state.last_y = map[center()].y;
    state.target_item_sprite = map[tile_id].it_sprite;
    state.target_character_id = map[tile_id].ch_nr;
    state.number_of_attempts = 0;
    state.next_attempt = get_next_attempt_time();
    state.indicator = false;
    state.last_combat_target = 0;
}

void mod_stubborn_actions_on_misc_action(int action) {
    if (state.pending_action == 0) return;
    if (action == DR_USE && state.pending_action == CL_CMD_USE) {
        state.pending_action = 0;
        state.indicator = false;
        pop_cmd_from_queue();
    } else if (
        (action == DR_GIVE && state.pending_action == CL_CMD_GIVE) ||
        (action == DR_PICKUP && state.pending_action == CL_CMD_DROP) || // No idea why the action is inverted for pickup/drop
        (action == DR_DROP && state.pending_action == CL_CMD_PICKUP)
    ) {
        state.pending_action = 0;
        state.indicator = false;
    }
}

bool mod_stubborn_actions_pending_use() {
    return state.pending_action == CL_CMD_USE;
}

void mod_stubborn_actions_clear() {
    state.indicator = false;
    state.pending_action = 0;
}

void mod_stubborn_actions_on_tick(const Uint32 delta_time) {
    if (state.pending_action) {
        if (state.next_attempt <= delta_time) {
            if (pl.attack_cn > 0) {
                if (pl.attack_cn != state.last_combat_target) {
                    state.last_combat_target = pl.attack_cn;
                    state.number_of_attempts = 0;
                }
                state.next_attempt = get_next_attempt_time();
                return;
            }
            switch (state.pending_action) {
                case CL_CMD_USE:
                    if (pl.misc_action == DR_USE) return;
                    break;
                case CL_CMD_DROP:
                    if (pl.misc_action == DR_DROP) return;
                    break;
                case CL_CMD_PICKUP:
                    if (pl.misc_action == DR_PICKUP) return;
                    break;
                case CL_CMD_GIVE:
                    if (pl.misc_action == DR_GIVE) return;
                    break;
                case CL_CMD_MOVE:
                    if (pl.goto_x > 0 && pl.goto_y > 0) return;
                    if (map[center()].x == state.target_x && map[center()].y == state.target_y) {
                        state.pending_action = 0;
                        return;
                    }
                default:
                    break;
            }
            if (state.number_of_attempts > 10) return;
            cmdq(state.pending_action, state.target_x, state.target_y);
            if (map[center()].x != state.last_x && map[center()].y != state.last_y) {
                state.number_of_attempts = 0;
            } else {
                state.number_of_attempts++;
            }
            state.next_attempt = get_next_attempt_time();
            state.indicator = true;
        } else {
            state.next_attempt -= delta_time;
        }
    }
}

bool mod_stubborn_actions_is_cmd_pending(int cmd, int x, int y) {
    return is_stubborn_action_enabled(cmd) && state.indicator && cmd == state.pending_action && x == state.target_x && y == state.target_y;
}
