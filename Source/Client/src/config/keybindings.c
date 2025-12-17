#include "keybindings.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "config.h"

/* Initialize default keybindings to match current hardcoded values */
void keybindings_init(void) {
    /* Spell 1-5: Ctrl+1-5 (buttons 16-20) */
    g_config.keybind.spell_hotkeys[0].key = SDLK_1;
    g_config.keybind.spell_hotkeys[0].modifier = KEYBIND_MOD_CTRL;

    g_config.keybind.spell_hotkeys[1].key = SDLK_2;
    g_config.keybind.spell_hotkeys[1].modifier = KEYBIND_MOD_CTRL;

    g_config.keybind.spell_hotkeys[2].key = SDLK_3;
    g_config.keybind.spell_hotkeys[2].modifier = KEYBIND_MOD_CTRL;

    g_config.keybind.spell_hotkeys[3].key = SDLK_4;
    g_config.keybind.spell_hotkeys[3].modifier = KEYBIND_MOD_CTRL;

    g_config.keybind.spell_hotkeys[4].key = SDLK_5;
    g_config.keybind.spell_hotkeys[4].modifier = KEYBIND_MOD_CTRL;

    /* Spell 6-10: Ctrl+Q-T (buttons 21-25) */
    g_config.keybind.spell_hotkeys[5].key = SDLK_q;
    g_config.keybind.spell_hotkeys[5].modifier = KEYBIND_MOD_CTRL;

    g_config.keybind.spell_hotkeys[6].key = SDLK_w;
    g_config.keybind.spell_hotkeys[6].modifier = KEYBIND_MOD_CTRL;

    g_config.keybind.spell_hotkeys[7].key = SDLK_e;
    g_config.keybind.spell_hotkeys[7].modifier = KEYBIND_MOD_CTRL;

    g_config.keybind.spell_hotkeys[8].key = SDLK_r;
    g_config.keybind.spell_hotkeys[8].modifier = KEYBIND_MOD_CTRL;

    g_config.keybind.spell_hotkeys[9].key = SDLK_t;
    g_config.keybind.spell_hotkeys[9].modifier = KEYBIND_MOD_CTRL;

    /* Spell 11-15: Ctrl+A-G (buttons 26-30) */
    g_config.keybind.spell_hotkeys[10].key = SDLK_a;
    g_config.keybind.spell_hotkeys[10].modifier = KEYBIND_MOD_CTRL;

    g_config.keybind.spell_hotkeys[11].key = SDLK_s;
    g_config.keybind.spell_hotkeys[11].modifier = KEYBIND_MOD_CTRL;

    g_config.keybind.spell_hotkeys[12].key = SDLK_d;
    g_config.keybind.spell_hotkeys[12].modifier = KEYBIND_MOD_CTRL;

    g_config.keybind.spell_hotkeys[13].key = SDLK_f;
    g_config.keybind.spell_hotkeys[13].modifier = KEYBIND_MOD_CTRL;

    g_config.keybind.spell_hotkeys[14].key = SDLK_g;
    g_config.keybind.spell_hotkeys[14].modifier = KEYBIND_MOD_CTRL;

    /* Spell 16-20: Ctrl+Z-B (buttons 31-35) */
    g_config.keybind.spell_hotkeys[15].key = SDLK_z;
    g_config.keybind.spell_hotkeys[15].modifier = KEYBIND_MOD_CTRL;

    g_config.keybind.spell_hotkeys[16].key = SDLK_x;
    g_config.keybind.spell_hotkeys[16].modifier = KEYBIND_MOD_CTRL;

    g_config.keybind.spell_hotkeys[17].key = SDLK_c;
    g_config.keybind.spell_hotkeys[17].modifier = KEYBIND_MOD_CTRL;

    g_config.keybind.spell_hotkeys[18].key = SDLK_v;
    g_config.keybind.spell_hotkeys[18].modifier = KEYBIND_MOD_CTRL;

    g_config.keybind.spell_hotkeys[19].key = SDLK_b;
    g_config.keybind.spell_hotkeys[19].modifier = KEYBIND_MOD_CTRL;

    /* Speed Hotkeys */
    g_config.keybind.general_hotkeys[SPEED_FAST_HOTKEY].key = SDLK_F1;
    g_config.keybind.general_hotkeys[SPEED_FAST_HOTKEY].modifier = KEYBIND_MOD_NONE;

    g_config.keybind.general_hotkeys[SPEED_NORMAL_HOTKEY].key = SDLK_F2;
    g_config.keybind.general_hotkeys[SPEED_NORMAL_HOTKEY].modifier = KEYBIND_MOD_NONE;

    g_config.keybind.general_hotkeys[SPEED_SLOW_HOTKEY].key = SDLK_F3;
    g_config.keybind.general_hotkeys[SPEED_SLOW_HOTKEY].modifier = KEYBIND_MOD_NONE;

    /* World Hotkeys */
    g_config.keybind.general_hotkeys[TOGGLE_PERCENT_HOTKEY].key = SDLK_F4;
    g_config.keybind.general_hotkeys[TOGGLE_PERCENT_HOTKEY].modifier = KEYBIND_MOD_NONE;

    g_config.keybind.general_hotkeys[TOGGLE_STAT_BASE_HOTKEY].key = SDLK_F5;
    g_config.keybind.general_hotkeys[TOGGLE_STAT_BASE_HOTKEY].modifier = KEYBIND_MOD_NONE;

    g_config.keybind.general_hotkeys[TOGGLE_HIDE_SPRITE_HOTKEY].key = SDLK_F6;
    g_config.keybind.general_hotkeys[TOGGLE_HIDE_SPRITE_HOTKEY].modifier = KEYBIND_MOD_NONE;

    g_config.keybind.general_hotkeys[TOGGLE_NAMES_HOTKEY].key = SDLK_F7;
    g_config.keybind.general_hotkeys[TOGGLE_NAMES_HOTKEY].modifier = KEYBIND_MOD_NONE;

    g_config.keybind.general_hotkeys[TOGGLE_HEALTH_BARS_HOTKEY].key = SDLK_F8;
    g_config.keybind.general_hotkeys[TOGGLE_HEALTH_BARS_HOTKEY].modifier = KEYBIND_MOD_NONE;

    /* Window Hotkeys */
    g_config.keybind.general_hotkeys[TOGGLE_OPTIONS_HOTKEY].key = SDLK_F9;
    g_config.keybind.general_hotkeys[TOGGLE_OPTIONS_HOTKEY].modifier = KEYBIND_MOD_NONE;

    g_config.keybind.general_hotkeys[TOGGLE_GAMMA_HOTKEY].key = SDLK_F10;
    g_config.keybind.general_hotkeys[TOGGLE_GAMMA_HOTKEY].modifier = KEYBIND_MOD_NONE;

    g_config.keybind.general_hotkeys[RESET_WINDOW_SIZE_HOTKEY].key = SDLK_F10;
    g_config.keybind.general_hotkeys[RESET_WINDOW_SIZE_HOTKEY].modifier = KEYBIND_MOD_SHIFT;

    /* General Hotkeys */
    g_config.keybind.general_hotkeys[FIGHTBACK_HOTKEY].key = SDLK_UNKNOWN;
    g_config.keybind.general_hotkeys[FIGHTBACK_HOTKEY].modifier = KEYBIND_MOD_CTRL;

    g_config.keybind.general_hotkeys[SWAP_GEAR_HOTKEY].key = SDLK_INSERT;
    g_config.keybind.general_hotkeys[SWAP_GEAR_HOTKEY].modifier = KEYBIND_MOD_NONE;

    g_config.keybind.general_hotkeys[SWAP_POSITION_HOTKEY].key = SDLK_UNKNOWN;
    g_config.keybind.general_hotkeys[SWAP_POSITION_HOTKEY].modifier = KEYBIND_MOD_CTRL;

    g_config.keybind.general_hotkeys[DISPLAY_DEBUG_INFORMATION_HOTKEY].key = SDLK_F11;
    g_config.keybind.general_hotkeys[DISPLAY_DEBUG_INFORMATION_HOTKEY].modifier = KEYBIND_MOD_NONE;

    g_config.keybind.general_hotkeys[EXIT_HOTKEY].key = SDLK_F12;
    g_config.keybind.general_hotkeys[EXIT_HOTKEY].modifier = KEYBIND_MOD_NONE;

    /* GC Hotkeys */
    g_config.keybind.general_hotkeys[GC_OFFENSE_HOTKEY].key = SDLK_UNKNOWN;
    g_config.keybind.general_hotkeys[GC_OFFENSE_HOTKEY].modifier = KEYBIND_MOD_CTRL;

    g_config.keybind.general_hotkeys[GC_DEFENSE_HOTKEY].key = SDLK_UNKNOWN;
    g_config.keybind.general_hotkeys[GC_DEFENSE_HOTKEY].modifier = KEYBIND_MOD_CTRL;

    g_config.keybind.general_hotkeys[GC_PASSIVE_HOTKEY].key = SDLK_UNKNOWN;
    g_config.keybind.general_hotkeys[GC_PASSIVE_HOTKEY].modifier = KEYBIND_MOD_CTRL;

    g_config.keybind.general_hotkeys[GC_WAIT_HOTKEY].key = SDLK_UNKNOWN;
    g_config.keybind.general_hotkeys[GC_WAIT_HOTKEY].modifier = KEYBIND_MOD_CTRL;

    g_config.keybind.general_hotkeys[GC_FOLLOW_HOTKEY].key = SDLK_UNKNOWN;
    g_config.keybind.general_hotkeys[GC_FOLLOW_HOTKEY].modifier = KEYBIND_MOD_CTRL;

    g_config.keybind.general_hotkeys[GC_MOVE_HOTKEY].key = SDLK_UNKNOWN;
    g_config.keybind.general_hotkeys[GC_MOVE_HOTKEY].modifier = KEYBIND_MOD_CTRL;

    g_config.keybind.general_hotkeys[GC_BUFFS_HOTKEY].key = SDLK_UNKNOWN;
    g_config.keybind.general_hotkeys[GC_BUFFS_HOTKEY].modifier = KEYBIND_MOD_CTRL;

}

/* Helper function to get key name */
static const char* get_key_name(SDL_Keycode key) {
    /* Handle common special keys */
    switch (key) {
        case SDLK_SPACE: return "Space";
        case SDLK_TAB: return "Tab";
        case SDLK_BACKSPACE: return "Backspace";
        case SDLK_INSERT: return "Insert";
        case SDLK_HOME: return "Home";
        case SDLK_END: return "End";
        case SDLK_PAGEUP: return "PgUp";
        case SDLK_PAGEDOWN: return "PgDn";
        case SDLK_UP: return "Up";
        case SDLK_DOWN: return "Down";
        case SDLK_LEFT: return "Left";
        case SDLK_RIGHT: return "Right";
        case SDLK_F1: return "F1";
        case SDLK_F2: return "F2";
        case SDLK_F3: return "F3";
        case SDLK_F4: return "F4";
        case SDLK_F5: return "F5";
        case SDLK_F6: return "F6";
        case SDLK_F7: return "F7";
        case SDLK_F8: return "F8";
        case SDLK_F9: return "F9";
        case SDLK_F10: return "F10";
        case SDLK_F11: return "F11";
        case SDLK_F12: return "F12";
        case SDLK_MINUS: return "-";
        case SDLK_EQUALS: return "=";
        case SDLK_LEFTBRACKET: return "[";
        case SDLK_RIGHTBRACKET: return "]";
        case SDLK_BACKSLASH: return "\\";
        case SDLK_SEMICOLON: return ";";
        case SDLK_QUOTE: return "'";
        case SDLK_COMMA: return ",";
        case SDLK_PERIOD: return ".";
        case SDLK_SLASH: return "/";
        case SDLK_BACKQUOTE: return "`";
        default:
            return SDL_GetKeyName(key);
    }
}

const char* keybinding_to_short_string(Keybinding kb, char* buffer, int buffer_size) {
    char mod_str[16] = "";

    /* Build modifier string */
    if (kb.modifier & KEYBIND_MOD_SHIFT) {
        strcat(mod_str, "S-");
    }
    if (kb.modifier & KEYBIND_MOD_CTRL) {
        strcat(mod_str, "C-");
    }
    if (kb.modifier & KEYBIND_MOD_ALT) {
        strcat(mod_str, "A-");
    }

    /* Get key name */
    const char* key_name = get_key_name(kb.key);

    /* For single letter keys, show uppercase */
    if (kb.key >= SDLK_a && kb.key <= SDLK_z) {
        char key_char[2];
        key_char[0] = (char)(kb.key - SDLK_a + 'A');
        key_char[1] = '\0';
        snprintf(buffer, buffer_size, "%s%s", mod_str, key_char);
    } else if (kb.key >= SDLK_1 && kb.key <= SDLK_9) {
        /* Number keys 1-9 */
        char key_char[2];
        key_char[0] = (char)(kb.key - SDLK_1 + '1');
        key_char[1] = '\0';
        snprintf(buffer, buffer_size, "%s%s", mod_str, key_char);
    } else if (kb.key == SDLK_0) {
        snprintf(buffer, buffer_size, "%s0", mod_str);
    } else {
        snprintf(buffer, buffer_size, "%s%s", mod_str, key_name);
    }

    return buffer;
}

/* Convert keybinding to display string like "Ctrl+A" */
const char* keybinding_to_string(Keybinding kb, char* buffer, int buffer_size) {
    char mod_str[32] = "";

    if (kb.key == SDLK_UNKNOWN) {
        snprintf(buffer, buffer_size, "Not Set");
        return buffer;
    }

    /* Build modifier string */
    if (kb.modifier & KEYBIND_MOD_SHIFT) {
        strcat(mod_str, "Shift+");
    }
    if (kb.modifier & KEYBIND_MOD_CTRL) {
        strcat(mod_str, "Ctrl+");
    }
    if (kb.modifier & KEYBIND_MOD_ALT) {
        strcat(mod_str, "Alt+");
    }

    /* Get key name */
    const char* key_name = get_key_name(kb.key);

    /* For single letter keys, show uppercase */
    if (kb.key >= SDLK_a && kb.key <= SDLK_z) {
        char key_char[2];
        key_char[0] = (char)(kb.key - SDLK_a + 'A');
        key_char[1] = '\0';
        snprintf(buffer, buffer_size, "%s%s", mod_str, key_char);
    } else if (kb.key >= SDLK_1 && kb.key <= SDLK_9) {
        /* Number keys 1-9 */
        char key_char[2];
        key_char[0] = (char)(kb.key - SDLK_1 + '1');
        key_char[1] = '\0';
        snprintf(buffer, buffer_size, "%s%s", mod_str, key_char);
    } else if (kb.key == SDLK_0) {
        snprintf(buffer, buffer_size, "%s0", mod_str);
    } else {
        snprintf(buffer, buffer_size, "%s%s", mod_str, key_name);
    }

    return buffer;
}

/* Find spell slot (0-19) matching key+modifier, returns -1 if no match */
int keybinding_find_spell_slot(SDL_Keycode key, int sdl_modstate) {
    int i;
    bool has_shift = (sdl_modstate & KMOD_SHIFT) != 0;
    bool has_ctrl = (sdl_modstate & KMOD_CTRL) != 0;
    bool has_alt = (sdl_modstate & KMOD_ALT) != 0;

    /* Build current modifier state */
    KeybindModifier current_mod = KEYBIND_MOD_NONE;
    if (has_shift) current_mod |= KEYBIND_MOD_SHIFT;
    if (has_ctrl) current_mod |= KEYBIND_MOD_CTRL;
    if (has_alt) current_mod |= KEYBIND_MOD_ALT;

    for (i = 0; i < NUM_SPELL_HOTKEYS; i++) {
        Keybinding kb = g_config.keybind.spell_hotkeys[i];

        /* Check key match */
        if (kb.key != key) continue;

        /* Check exact modifier match */
        if (kb.modifier == current_mod) return i;
    }

    return -1;  /* No match found */
}

int keybinding_find_general(SDL_KeyCode key, int sdl_modstate) {
    int i;
    bool has_shift = (sdl_modstate & KMOD_SHIFT) != 0;
    bool has_ctrl = (sdl_modstate & KMOD_CTRL) != 0;
    bool has_alt = (sdl_modstate & KMOD_ALT) != 0;

    /* Build current modifier state */
    KeybindModifier current_mod = KEYBIND_MOD_NONE;
    if (has_shift) current_mod |= KEYBIND_MOD_SHIFT;
    if (has_ctrl) current_mod |= KEYBIND_MOD_CTRL;
    if (has_alt) current_mod |= KEYBIND_MOD_ALT;

    for (i = 0; i < NUM_GENERAL_HOTKEYS; i++) {
        Keybinding kb = g_config.keybind.general_hotkeys[i];
        if (kb.key != key) continue;

        /* Check exact modifier match */
        if (kb.modifier == current_mod) return i;
    }

    return -1;
}
