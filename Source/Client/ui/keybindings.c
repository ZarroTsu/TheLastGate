#include "keybindings.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/* Global keybinding configuration */
KeybindConfig keybind_config;

/* Initialize default keybindings to match current hardcoded values */
void keybindings_init(void) {
    /* Spell 1-5: Ctrl+1-5 (buttons 16-20) */
    keybind_config.spell_hotkeys[0].key = SDLK_1;
    keybind_config.spell_hotkeys[0].modifier = KEYBIND_MOD_CTRL;

    keybind_config.spell_hotkeys[1].key = SDLK_2;
    keybind_config.spell_hotkeys[1].modifier = KEYBIND_MOD_CTRL;

    keybind_config.spell_hotkeys[2].key = SDLK_3;
    keybind_config.spell_hotkeys[2].modifier = KEYBIND_MOD_CTRL;

    keybind_config.spell_hotkeys[3].key = SDLK_4;
    keybind_config.spell_hotkeys[3].modifier = KEYBIND_MOD_CTRL;

    keybind_config.spell_hotkeys[4].key = SDLK_5;
    keybind_config.spell_hotkeys[4].modifier = KEYBIND_MOD_CTRL;

    /* Spell 6-10: Ctrl+Q-T (buttons 21-25) */
    keybind_config.spell_hotkeys[5].key = SDLK_q;
    keybind_config.spell_hotkeys[5].modifier = KEYBIND_MOD_CTRL;

    keybind_config.spell_hotkeys[6].key = SDLK_w;
    keybind_config.spell_hotkeys[6].modifier = KEYBIND_MOD_CTRL;

    keybind_config.spell_hotkeys[7].key = SDLK_e;
    keybind_config.spell_hotkeys[7].modifier = KEYBIND_MOD_CTRL;

    keybind_config.spell_hotkeys[8].key = SDLK_r;
    keybind_config.spell_hotkeys[8].modifier = KEYBIND_MOD_CTRL;

    keybind_config.spell_hotkeys[9].key = SDLK_t;
    keybind_config.spell_hotkeys[9].modifier = KEYBIND_MOD_CTRL;

    /* Spell 11-15: Ctrl+A-G (buttons 26-30) */
    keybind_config.spell_hotkeys[10].key = SDLK_a;
    keybind_config.spell_hotkeys[10].modifier = KEYBIND_MOD_CTRL;

    keybind_config.spell_hotkeys[11].key = SDLK_s;
    keybind_config.spell_hotkeys[11].modifier = KEYBIND_MOD_CTRL;

    keybind_config.spell_hotkeys[12].key = SDLK_d;
    keybind_config.spell_hotkeys[12].modifier = KEYBIND_MOD_CTRL;

    keybind_config.spell_hotkeys[13].key = SDLK_f;
    keybind_config.spell_hotkeys[13].modifier = KEYBIND_MOD_CTRL;

    keybind_config.spell_hotkeys[14].key = SDLK_g;
    keybind_config.spell_hotkeys[14].modifier = KEYBIND_MOD_CTRL;

    /* Spell 16-20: Ctrl+Z-B (buttons 31-35) */
    keybind_config.spell_hotkeys[15].key = SDLK_z;
    keybind_config.spell_hotkeys[15].modifier = KEYBIND_MOD_CTRL;

    keybind_config.spell_hotkeys[16].key = SDLK_x;
    keybind_config.spell_hotkeys[16].modifier = KEYBIND_MOD_CTRL;

    keybind_config.spell_hotkeys[17].key = SDLK_c;
    keybind_config.spell_hotkeys[17].modifier = KEYBIND_MOD_CTRL;

    keybind_config.spell_hotkeys[18].key = SDLK_v;
    keybind_config.spell_hotkeys[18].modifier = KEYBIND_MOD_CTRL;

    keybind_config.spell_hotkeys[19].key = SDLK_b;
    keybind_config.spell_hotkeys[19].modifier = KEYBIND_MOD_CTRL;

    /* General Hotkeys */
    keybind_config.general_hotkeys[FIGHTBACK_HOTKEY].key = SDLK_UNKNOWN;
    keybind_config.general_hotkeys[FIGHTBACK_HOTKEY].modifier = KEYBIND_MOD_CTRL;

    keybind_config.general_hotkeys[SWAP_HOTKEY].key = SDLK_UNKNOWN;
    keybind_config.general_hotkeys[SWAP_HOTKEY].modifier = KEYBIND_MOD_CTRL;

    keybind_config.general_hotkeys[GC_OFFENSE_HOTKEY].key = SDLK_UNKNOWN;
    keybind_config.general_hotkeys[GC_OFFENSE_HOTKEY].modifier = KEYBIND_MOD_CTRL;

    keybind_config.general_hotkeys[GC_DEFENSE_HOTKEY].key = SDLK_UNKNOWN;
    keybind_config.general_hotkeys[GC_DEFENSE_HOTKEY].modifier = KEYBIND_MOD_CTRL;

    keybind_config.general_hotkeys[GC_PASSIVE_HOTKEY].key = SDLK_UNKNOWN;
    keybind_config.general_hotkeys[GC_PASSIVE_HOTKEY].modifier = KEYBIND_MOD_CTRL;

    keybind_config.general_hotkeys[GC_WAIT_HOTKEY].key = SDLK_UNKNOWN;
    keybind_config.general_hotkeys[GC_WAIT_HOTKEY].modifier = KEYBIND_MOD_CTRL;

    keybind_config.general_hotkeys[GC_FOLLOW_HOTKEY].key = SDLK_UNKNOWN;
    keybind_config.general_hotkeys[GC_FOLLOW_HOTKEY].modifier = KEYBIND_MOD_CTRL;

    keybind_config.general_hotkeys[GC_MOVE_HOTKEY].key = SDLK_UNKNOWN;
    keybind_config.general_hotkeys[GC_MOVE_HOTKEY].modifier = KEYBIND_MOD_CTRL;

    keybind_config.general_hotkeys[GC_BUFFS_HOTKEY].key = SDLK_UNKNOWN;
    keybind_config.general_hotkeys[GC_BUFFS_HOTKEY].modifier = KEYBIND_MOD_CTRL;

}

const char* keybinding_to_short_string(Keybinding kb, char* buffer, int buffer_size) {
    const char* mod_str = "";
    char key_char[2];

    /* Get modifier string */
    if (kb.modifier == KEYBIND_MOD_CTRL) {
        mod_str = "C-";
    } else if (kb.modifier == KEYBIND_MOD_ALT) {
        mod_str = "A-";
    }

    /* Get key string - handle common keys */
    if (kb.key >= SDLK_a && kb.key <= SDLK_z) {
        /* Letter keys - display as uppercase */
        key_char[0] = (char)(kb.key - SDLK_a + 'A');
        key_char[1] = '\0';
        snprintf(buffer, buffer_size, "%s%s", mod_str, key_char);
    } else if (kb.key >= SDLK_1 && kb.key <= SDLK_9) {
        /* Number keys 1-9 */
        key_char[0] = (char)(kb.key - SDLK_1 + '1');
        key_char[1] = '\0';
        snprintf(buffer, buffer_size, "%s%s", mod_str, key_char);
    } else if (kb.key == SDLK_0) {
        /* Number key 0 */
        snprintf(buffer, buffer_size, "%s0", mod_str);
    } else {
        /* Unknown key */
        snprintf(buffer, buffer_size, "%s?", mod_str);
    }

    return buffer;
}

/* Convert keybinding to display string like "Ctrl+A" */
const char* keybinding_to_string(Keybinding kb, char* buffer, int buffer_size) {
    const char* mod_str = "";
    char key_char[2];

    if (kb.key == SDLK_UNKNOWN) {
        snprintf(buffer, buffer_size, "Not Set");
        return buffer;
    }

    /* Get modifier string */
    if (kb.modifier == KEYBIND_MOD_CTRL) {
        mod_str = "Ctrl+";
    } else if (kb.modifier == KEYBIND_MOD_ALT) {
        mod_str = "Alt+";
    }

    /* Get key string - handle common keys */
    if (kb.key >= SDLK_a && kb.key <= SDLK_z) {
        /* Letter keys - display as uppercase */
        key_char[0] = (char)(kb.key - SDLK_a + 'A');
        key_char[1] = '\0';
        snprintf(buffer, buffer_size, "%s%s", mod_str, key_char);
    } else if (kb.key >= SDLK_1 && kb.key <= SDLK_9) {
        /* Number keys 1-9 */
        key_char[0] = (char)(kb.key - SDLK_1 + '1');
        key_char[1] = '\0';
        snprintf(buffer, buffer_size, "%s%s", mod_str, key_char);
    } else if (kb.key == SDLK_0) {
        /* Number key 0 */
        snprintf(buffer, buffer_size, "%s0", mod_str);
    } else {
        /* Unknown key */
        snprintf(buffer, buffer_size, "%s?", mod_str);
    }

    return buffer;
}

/* Find spell slot (0-19) matching key+modifier, returns -1 if no match */
int keybinding_find_spell_slot(SDL_Keycode key, int sdl_modstate) {
    int i;
    bool has_ctrl = (sdl_modstate & KMOD_CTRL) != 0;
    bool has_alt = (sdl_modstate & KMOD_ALT) != 0;

    for (i = 0; i < NUM_SPELL_HOTKEYS; i++) {
        Keybinding kb = keybind_config.spell_hotkeys[i];

        /* Check key match */
        if (kb.key != key) continue;

        /* Check modifier match (lenient - ignore Shift, Caps, etc.) */
        if (kb.modifier == KEYBIND_MOD_CTRL) {
            if (has_ctrl && !has_alt) return i;  /* Require Ctrl, forbid Alt */
        } else if (kb.modifier == KEYBIND_MOD_ALT) {
            if (has_alt && !has_ctrl) return i;  /* Require Alt, forbid Ctrl */
        }
        /* KEYBIND_MOD_NONE: Skip unbound entries */
    }

    return -1;  /* No match found */
}

int keybinding_find_general(SDL_KeyCode key, int sdl_modstate) {
    int i;
    bool has_ctrl = (sdl_modstate & KMOD_CTRL) != 0;
    bool has_alt = (sdl_modstate & KMOD_ALT) != 0;

    for (i = 0; i < NUM_GENERAL_HOTKEYS; i++) {
        Keybinding kb = keybind_config.general_hotkeys[i];
        if (kb.key != key) continue;

        if (kb.modifier == KEYBIND_MOD_CTRL) {
            if (has_ctrl && !has_alt) return i;
        } else if (kb.modifier == KEYBIND_MOD_ALT) {
            if (has_alt && !has_ctrl) return i;
        }
    }

    return -1;
}
