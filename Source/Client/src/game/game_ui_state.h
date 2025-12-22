#pragma once
#include <stdbool.h>

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
    int waypoint_scroll;

    /* Hud Mode */
    GameHudMode hud_mode;

    /* Window Visibility */
    unsigned int open_shop;
    bool show_waypoints;
    unsigned int open_skill_tree;
    bool show_options;
    unsigned int open_book;
    bool show_motd;
    bool show_new_player;
    TutorialState tutorial;

} GameUiState;