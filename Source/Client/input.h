#pragma once

extern int quit;

void init_input(void);

void handle_input(void);

typedef enum {
    NONE,
    INVENTORY,
    SKILL_LIST,
    WAYPOINT_PAGE,
    DEPOT_PAGE,
    OPTIONS_WINDOW,
    CHAT_HISTORY
} ScrollableRegion;
