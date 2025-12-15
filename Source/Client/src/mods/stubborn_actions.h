#pragma once
#include <stdbool.h>
#include <SDL2/SDL_stdinc.h>

typedef struct {
    int pending_action;
    int target_x;
    int target_y;
    int last_x;
    int last_y;
    Uint32 next_attempt;
    int number_of_attempts;
    int target_item_sprite; // Used for DROP/PICKUP
    int target_character_id; // Used for GIVE
} StubbornActionState;

void mod_stubborn_actions_on_cmd(int action, int tile_id);
void mod_stubborn_actions_on_misc_action(int action);
bool mod_stubborn_actions_pending_use();
void mod_stubborn_actions_clear();
void mod_stubborn_actions_on_tick(Uint32 delta_time);