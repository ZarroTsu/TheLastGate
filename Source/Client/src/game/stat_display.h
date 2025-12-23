#pragma once
#include <stdbool.h>

typedef struct {
    const char *name;
    int base_value;
    int current_value;
    int raised_amount;
    int cost_to_raise;
    char raise_icon;
    bool can_lower;
    bool is_visible;
    int text_color;
} StatDisplayInfo;

/* Build Helpers */
void get_attribute_display_info(int attrib_id, StatDisplayInfo *out);

void get_hp_mana_display_info(int vital_id, StatDisplayInfo *out);

/* Render Helpers */
void render_stat_line(const StatDisplayInfo *stat, int x, int y, bool show_base_stats);
