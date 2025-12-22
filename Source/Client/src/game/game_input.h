#pragma once
#include <stdbool.h>
#include <SDL2/SDL_events.h>

extern bool waiting_for_keybind;

void game_input_init(void);
void game_input_shutdown(void);

void sync_chat_input_state(void);

void game_handle_input(const SDL_Event *e);

typedef enum {
    NONE,
    INVENTORY,
    SKILL_LIST,
    WAYPOINT_PAGE,
    DEPOT_PAGE,
    OPTIONS_WINDOW,
    CHAT_HISTORY
} ScrollableRegion;
