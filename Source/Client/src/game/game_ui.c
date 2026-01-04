//
// Created by james on 12/21/2025.
//

#include "game_ui.h"

#include "engine.h"
#include "inter.h"
#include "ui/states/perf_window_state.h"
#include "util/perf.h"

GameUiState game_ui_state = {
    /* Scroll Positions */
    .inventory_scroll = 0,
    .skill_scroll = 0,
    .waypoint_scroll = 0,

    /* Hud Mode */
    .hud_mode = HUD_MODE_LIST_SKILLS,

    /* Window Settings */
    .tutorial = {
        .page = 0,
        .open = 0,
        .count = 0
    },

    .show_waypoints = false,
    .show_options = false,
    .show_motd = false,
    .show_new_player = false,

    .open_shop = 0,
    .open_skill_tree = 0,
    .open_book = 0,
    .open_depot_page = 0,

    /* Minimap */
    .minimap_magnification = 2,
};

void ui_calculate_perf_window_state() {
    const PerfStats *stats = perf_get_stats();
    game_ui_state.perf_window_state.fps_avg = stats->fps_avg;
    game_ui_state.perf_window_state.sample_ms_min = stats->sample_ms_min;
    game_ui_state.perf_window_state.sample_ms_max = stats->sample_ms_max;
}

static const int META_HIT_SCORE_ID = 5;
static const int META_PARRY_SCORE_ID = 6;

void ui_calculate_quick_stats_window_state(void) {
    game_ui_state.quick_stat_state.weapon_value = pl.weapon;
    game_ui_state.quick_stat_state.armor_value = pl.armor;
    game_ui_state.quick_stat_state.hit_score = meta_stats[META_HIT_SCORE_ID].value;
    game_ui_state.quick_stat_state.parry_score = meta_stats[META_PARRY_SCORE_ID].value;
    game_ui_state.quick_stat_state.experience = pl.points_tot;

    if (points2rank(pl.points_tot) == MAX_RANK) {
        game_ui_state.quick_stat_state.experience_bar_progress = 1.0F;
    } else {
        float starting_experience = (float) rank2points(points2rank(pl.points_tot));
        float ending_experience = (float) pl.points_tot + (float) points_tolevel(pl.points_tot);
        float percentage = ((float) pl.points_tot - starting_experience) / (ending_experience - starting_experience);
        game_ui_state.quick_stat_state.experience_bar_progress = percentage;
    }
}
