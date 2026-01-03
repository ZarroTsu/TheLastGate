#include "stat_display.h"

#include <stddef.h>

#include "common.h"
#include "engine.h"
#include "game_ui.h"
#include "inter.h"
#include "config/config.h"
#include "graphics/render.h"
#include "mods/base_cost_warning.h"
#include "util/math_util.h"

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
    if (vital_id == 5) {
        // Health
        out->name = "Hitpoints";
        out->base_value = pl.hp[0] + stat_raised[5];
        out->current_value = pl.hp[5] + stat_raised[5];
        out->cost_to_raise = hp_needed(pl.hp[0] + stat_raised[5]);
    } else if (vital_id == 7) {
        // Mana
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

/* Returns true if the skill is always known */
/* Stealth, Resist, Regen, Rest, Meditate, Immunity */
static bool is_inherit_skill(int skill_id) {
    return skill_id == 8 || skill_id == 23 || skill_id == 28 || skill_id == 29 || skill_id == 30 || skill_id == 32 || (
               skill_id == 44 && IS_SEYAN_DU);
}

static bool is_alternate(int skill_id) {
    const int pl_flags = pl.worn[WN_FLAGS];
    const int pl_flagb = pl.worn_p[WN_FLAGS];

    return (skill_id == 11 && (pl_flagb & (1 << 10))) || // Magic Shield -> Magic Shell
           (skill_id == 19 && (pl_flags & (1 << 5))) || // Slow -> Greater Slow
           (skill_id == 20 && (pl_flags & (1 << 6))) || // Curse -> Greater Curse
           (skill_id == 24 && (pl_flags & (1 << 7))) || // Blast -> +Scorch
           (skill_id == 26 && (pl_flags & (1 << 14))) || // Heal -> Regen
           (skill_id == 37 && (pl_flagb & (1 << 11))) || // Blind -> Douse
           (skill_id == 40 && (pl_flags & (1 << 8))) || // Cleave -> +Aggravate
           (skill_id == 41 && (pl_flags & (1 << 10))) || // Weaken -> Greater Weaken
           (skill_id == 16 && (pl_flagb & (1 << 5))) || // Shield -> Shield Bash
           (skill_id == 43 && (pl_flagb & (1 << 6))) || // Pulse -> Healing Pulses
           (skill_id == 49 && (pl_flagb & (1 << 7))) || // Leap
           (skill_id == 35 && (pl_flagb & (1 << 12))) || // Warcry -> Rally
           (skill_id == 42 && (pl_flagb & (1 << 14))) || // Poison -> Venom
           (skill_id == 12 && (pl_flagb & (1 << 3))) || // Tactics invert
           (skill_id == 22 && IS_SHIFTED); // Rage -> Calm;
}

void get_skill_display_info(const int skill_slot, StatDisplayInfo *out) {
    int skill_tab_id = skill_slot + game_ui_state.skill_scroll;
    int skill_position = 8 + skill_tab_id;
    struct skilltab skill_tab = skilltab[skill_tab_id];
    int skill_id = skill_tab.nr;
    int amount_raised = stat_raised[skill_position];

    if (skill_tab.show < 1) {
        return;
    }

    out->is_visible = true;
    out->name = skill_tab.name;

    if (skill_tab.show != 1) {
        // Not Known
        if (skill_tab.show == 4) {
            // Show in red
            out->current_value = sk_score(skill_id);
            out->text_color = 0;
            return;
        }

        if (skill_tab.show == 2) {
            out->current_value = max(300, max(1, (points2rank(pl.points_tot) + 1) * 8));
            out->text_color = 5;
            return;
        }
    }

    out->base_value = pl.skill[skill_id][0] + amount_raised;
    out->current_value = sk_score(skill_id) + amount_raised;
    out->cost_to_raise = skill_needed(skill_id, pl.skill[skill_id][0] + amount_raised);
    if (g_config.ui.cost_helper) {
        out->raise_icon = get_raise_icon(skill_id, skill_tab_id);
    } else {
        out->raise_icon = out->cost_to_raise <= pl.points - stat_points_used ? '+' : ' ';
    }
    out->can_lower = amount_raised > 0;
    out->text_color = 1;
}

void render_stat_line(const StatDisplayInfo *stat, int x, int y, bool show_base_stats) {
    if (stat->is_visible) {
        xputtext(x + 9, y, stat->text_color, "%-20.20s", stat->name);
    } else {
        xputtext(x + 9, y, 1, "-");
    }
    if (show_base_stats && stat->base_value > 0) xputtext(x + 117, y, 3, "%3d", stat->base_value);
    if (stat->is_visible) {
        xputtext(x + 140, y, stat->text_color, "%3d", stat->current_value);
    }
    render_putc(x + 163, y, g_config.ui.cost_helper && stat->raise_icon != '+' ? 2 : 1, stat->raise_icon);
    if (stat->can_lower)
        render_putc(x + 177, y, 1, '-');
    if (stat->cost_to_raise != HIGH_VAL && stat->base_value > 0)
        xputtext(x + 189, y, 1, "%7d", stat->cost_to_raise);
}
