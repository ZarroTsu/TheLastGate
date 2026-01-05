#pragma once

typedef struct {
    int weapon_value;
    int armor_value;
    int hit_score;
    int parry_score;
    unsigned int experience;
    float experience_bar_progress;
} QuickStatState;

void quick_stats_render(QuickStatState *state);