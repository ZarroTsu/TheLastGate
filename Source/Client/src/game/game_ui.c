//
// Created by james on 12/21/2025.
//

#include "game_ui.h"

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