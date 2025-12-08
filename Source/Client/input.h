#pragma once
#include <SDL2/SDL_events.h>

extern int quit;
extern SDL_Event input_events[128];
extern int input_event_count;

void init_input(void);

void handle_input(void);

void scale_event_for_imgui(SDL_Event *event);

typedef enum {
    NONE,
    INVENTORY,
    SKILL_LIST,
    WAYPOINT_PAGE,
    DEPOT_PAGE,
    OPTIONS_WINDOW,
    CHAT_HISTORY
} ScrollableRegion;
