#include "stat_display.h"

#include <stddef.h>

#include "common.h"
#include "engine.h"
#include "inter.h"
#include "graphics/render.h"

void get_attribute_display_info(const int attrib_id, StatDisplayInfo *out) {
    if (out == NULL) return;
    out->name = at_name[attrib_id];
    out->base_value = pl.attrib[attrib_id][0] + stat_raised[attrib_id];
    out->current_value = at_score(attrib_id) + stat_raised[attrib_id];
    out->raised_amount = stat_raised[attrib_id];
    out->cost_to_raise = attrib_needed(attrib_id, pl.attrib[attrib_id][0] + stat_raised[attrib_id]);
    out->raise_icon = out->cost_to_raise <= pl.points - stat_points_used ? '+' : ' ';
    out->can_lower = stat_raised[attrib_id] > 0;
    out->is_visible = true;
    out->text_color = 1;
}

void get_hp_mana_display_info(const int vital_id, StatDisplayInfo *out) {
    if (vital_id == 5) { // Health
        out->name = "Hitpoints";
        out->base_value = pl.hp[0] + stat_raised[5];
        out->current_value = pl.hp[5] + stat_raised[5];
        out->cost_to_raise = hp_needed(pl.hp[0] + stat_raised[5]);
    } else if (vital_id == 7) { // Mana
        out->name = "Mana";
        out->base_value = pl.mana[0] + stat_raised[7];
        out->current_value = pl.mana[5] + stat_raised[7];
        out->cost_to_raise = mana_needed(pl.mana[0] + stat_raised[7]);
    }

    out->raised_amount = stat_raised[vital_id];
    out->raise_icon = out->cost_to_raise <= pl.points - stat_points_used ? '+' : ' ';
    out->can_lower = stat_raised[vital_id] > 0;
    out->is_visible = true;
    out->text_color = 1;
}

void render_stat_line(const StatDisplayInfo *stat, int x, int y, bool show_base_stats) {
    xputtext(x + 9, y, 1, "%-20.20s", stat->name);
    if (show_base_stats) xputtext(x + 117, y, 3, "%3d", stat->base_value);
    xputtext(x + 140, y, 1, "%3d", stat->current_value);
    render_putc(x + 163, y, 1, stat->raise_icon);
    if (stat->can_lower)
        render_putc(x + 177, y, 1, '-');
    if (stat->cost_to_raise != HIGH_VAL)
        xputtext(x + 189, y, 1, "%7d", stat->cost_to_raise);
}
