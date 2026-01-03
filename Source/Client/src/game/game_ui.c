//
// Created by james on 12/21/2025.
//

#include "game_ui.h"
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
    .minimap_magnification = 2
};

static PerfWindowState g_perf_window_state = {0};

const PerfWindowState *ui_get_perf_window_state() {
    const PerfStats *stats = perf_get_stats();
    g_perf_window_state.fps_avg = stats->fps_avg;
    g_perf_window_state.sample_ms_min = stats->sample_ms_min;
    g_perf_window_state.sample_ms_max = stats->sample_ms_max;
    return &g_perf_window_state;
}
