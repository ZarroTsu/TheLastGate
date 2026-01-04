#pragma once
#include <stdbool.h>

#include "ui/quick_stats.h"
#include "ui/states/perf_window_state.h"

typedef enum {
    HUD_MODE_LIST_SKILLS,
    HUD_MODE_LIST_OFFENSES,
    HUD_MODE_LIST_DEFENSES,
    HUD_MODE_LIST_SKILLS_AND_META
} GameHudMode;

typedef struct {
    unsigned int open;
    unsigned int page;
    unsigned int count;
} TutorialState;

typedef struct {
    /* Scroll Positions */
    int inventory_scroll;
    int skill_scroll;
    int meta_scroll;
    int waypoint_scroll;

    /* Hud Mode */
    GameHudMode hud_mode;

    /* Window Settings */
    TutorialState tutorial;

    bool show_waypoints;
    bool show_options;
    bool show_motd;
    bool show_new_player;

    unsigned int open_shop;
    unsigned int open_skill_tree;
    unsigned int open_book;
    unsigned int open_depot_page;

    /* Minimap */
    int minimap_magnification;

    /* Quick State Window (WV, AV, HS, PS Exp) */
    QuickStatState quick_stat_state;
    PerfWindowState perf_window_state;
} GameUiState;

void game_ui_state_calculate(void);
