#include "base_cost_warning.h"

#include <stdbool.h>

#include "common.h"
#include "engine.h"
#include "inter.h"

typedef struct {
    int cost;
    int attribute_targets[5];
} AttributeCostResult;

#define ATTRIBUTE_TARGETS_FROM_INITIAL_VALUES(iv) \
    { (iv)[0], (iv)[1], (iv)[2], (iv)[3], (iv)[4] }

static bool is_weapon_skill(int skill_id) {
    switch (skill_id) {
        case SK_HAND:
        case SK_AXE:
        case SK_DAGGER:
        case SK_STAFF:
        case SK_SWORD:
        case SK_TWOHAND:
            return true;
        default:
            return false;
    }
}

static bool is_spell(int skill_id) {
    switch (skill_id) {
        case SK_BLAST:
        case SK_BLESS:
        case SK_CURSE:
        case SK_DISPEL:
        case SK_DISPEL2:
        case SK_ENHANCE:
        case SK_GHOST:
        case SK_HASTE:
        case SK_HEAL:
        case SK_MSHIELD:
        case SK_POISON:
        case SK_PROTECT:
        case SK_PULSE:
        case SK_SHADOW:
        case SK_SLOW:
            return true;
        default:
            return false;
    }
}

static AttributeCostResult cost_to_raise_attributes_by_5(const int attributes[3]) {
    int initial_values[5] = {pl.attrib[0][0], pl.attrib[1][0], pl.attrib[2][0], pl.attrib[3][0], pl.attrib[4][0]};
    int total_cost = 0;
    int startingValue = (initial_values[attributes[0]] + initial_values[attributes[1]] + initial_values[attributes[2]])
                        / 5;
    int attempts = 0;

    while (attempts < 10) {
        int lowest_cost_index = -1;
        int lowest_cost = HIGH_VAL;
        for (int i = 0; i < 3; i++) {
            int a = attributes[i];
            int c = attrib_needed(a, initial_values[a]);
            if (c < lowest_cost) {
                lowest_cost = c;
                lowest_cost_index = i;
            }
        }
        int current_attribute = attributes[lowest_cost_index];
        int cost = lowest_cost;
        if (cost == HIGH_VAL) {
            break;
        }
        total_cost += cost;
        initial_values[current_attribute] += 1;
        int current_value = (initial_values[attributes[0]] + initial_values[attributes[1]] + initial_values[attributes[
                                 2]]) / 5;
        if (current_value > startingValue)
            return (AttributeCostResult){
                total_cost, ATTRIBUTE_TARGETS_FROM_INITIAL_VALUES(initial_values)
            };
        attempts++;
    }
    return (AttributeCostResult){HIGH_VAL, ATTRIBUTE_TARGETS_FROM_INITIAL_VALUES(initial_values)};
}

static AttributeCostResult cost_to_raise_attributes_by_5_uber(const int attributes[4]) {
    int initial_values[5] = {pl.attrib[0][0], pl.attrib[1][0], pl.attrib[2][0], pl.attrib[3][0], pl.attrib[4][0]};
    int total_cost = 0;
    int startingValue = ((initial_values[attributes[1]] + initial_values[attributes[2]]) / 2 + initial_values[attributes
                             [0]] + initial_values[attributes[0]]) / 5;
    int attempts = 0;

    while (attempts < 10) {
        int lowest_cost_index = -1;
        int lowest_cost = HIGH_VAL;
        for (int i = 0; i < 4; i++) {
            int a = attributes[i];
            int c = attrib_needed(a, initial_values[a]);
            if (c < lowest_cost) {
                lowest_cost = c;
                lowest_cost_index = i;
            }
        }
        int current_attribute = attributes[lowest_cost_index];
        int cost = lowest_cost;
        if (cost == HIGH_VAL) {
            break;
        }
        total_cost += cost;
        initial_values[current_attribute] += 1;

        int current_value = ((initial_values[attributes[1]] + initial_values[attributes[2]]) / 2 + initial_values[
                                 attributes[0]] + initial_values[attributes[0]]) / 5;
        if (current_value > startingValue)
            return (AttributeCostResult){
                total_cost, ATTRIBUTE_TARGETS_FROM_INITIAL_VALUES(initial_values)
            };
        attempts++;
    }
    return (AttributeCostResult){HIGH_VAL, ATTRIBUTE_TARGETS_FROM_INITIAL_VALUES(initial_values)};
}

char get_raise_icon(int skill_id, int skill_tab_position) {
    // Maxed
    if (skill_needed(skill_id, pl.skill[skill_id][0] + stat_raised[skill_tab_position + 8]) == HIGH_VAL) return ' ';

    int cost_to_raise_skill = skill_needed(skill_id, pl.skill[skill_id][0] + stat_raised[skill_tab_position + 8]);

    AttributeCostResult result;
    if (is_weapon_skill(skilltab[skill_tab_position].nr) && T_BRAV_SK(9)) {
        result = cost_to_raise_attributes_by_5_uber((int[4]){AT_BRV, AT_BRV, AT_AGL, AT_STR});
    } else if (is_spell(skilltab[skill_tab_position].nr) && T_ARHR_SK(9)) {
        result = cost_to_raise_attributes_by_5_uber((int[4]){AT_INT, AT_INT, AT_WIL, AT_BRV});
    } else {
        result = cost_to_raise_attributes_by_5(skilltab[skill_tab_position].attrib);
    }

    if (result.cost < cost_to_raise_skill) {
        if (pl.attrib[0][0] + stat_raised[0] < result.attribute_targets[0]) return 'B';
        if (pl.attrib[1][0] + stat_raised[1] < result.attribute_targets[1]) return 'W';
        if (pl.attrib[2][0] + stat_raised[2] < result.attribute_targets[2]) return 'I';
        if (pl.attrib[3][0] + stat_raised[3] < result.attribute_targets[3]) return 'A';
        if (pl.attrib[4][0] + stat_raised[4] < result.attribute_targets[4]) return 'S';
        return '*';
    }
    // Cannot Raise
    if (skill_needed(skill_id,pl.skill[skill_id][0]+stat_raised[skill_tab_position + 8])>pl.points-stat_points_used) return ' ';
    return '+';
}
