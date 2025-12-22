#pragma once

typedef enum {
    HUD_MODE_LIST_SKILLS,
    HUD_MODE_LIST_OFFENSES,
    HUD_MODE_LIST_DEFENSES,
    HUD_MODE_LIST_SKILLS_AND_META
} GameHudMode;

typedef struct {
    /* Scroll Positions */
    int inventory_scroll;
    int skill_scroll;
    int waypoint_scroll;

    /* Hud Mode */
    GameHudMode hud_mode;
} GameUiState;