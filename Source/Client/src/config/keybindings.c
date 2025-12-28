#include "keybindings.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "config.h"

static BindingDescriptor binding_registry[] = {
    /* Spell 1-5: Ctrl+1-5 (buttons 16-20) */
    {"Spell 1", "spell_1", BINDING_CATEGORY_SPELL, {SDLK_1, KEYBIND_MOD_CTRL}, {SDLK_1, KEYBIND_MOD_CTRL}},
    {"Spell 2", "spell_2", BINDING_CATEGORY_SPELL, {SDLK_2, KEYBIND_MOD_CTRL}, {SDLK_2, KEYBIND_MOD_CTRL}},
    {"Spell 3", "spell_3", BINDING_CATEGORY_SPELL, {SDLK_3, KEYBIND_MOD_CTRL}, {SDLK_3, KEYBIND_MOD_CTRL}},
    {"Spell 4", "spell_4", BINDING_CATEGORY_SPELL, {SDLK_4, KEYBIND_MOD_CTRL}, {SDLK_4, KEYBIND_MOD_CTRL}},
    {"Spell 5", "spell_5", BINDING_CATEGORY_SPELL, {SDLK_5, KEYBIND_MOD_CTRL}, {SDLK_5, KEYBIND_MOD_CTRL}},

    /* Spell 6-10: Ctrl+Q-T (buttons 21-25) */
    {"Spell 6", "spell_6", BINDING_CATEGORY_SPELL, {SDLK_q, KEYBIND_MOD_CTRL}, {SDLK_q, KEYBIND_MOD_CTRL}},
    {"Spell 7", "spell_7", BINDING_CATEGORY_SPELL, {SDLK_w, KEYBIND_MOD_CTRL}, {SDLK_w, KEYBIND_MOD_CTRL}},
    {"Spell 8", "spell_8", BINDING_CATEGORY_SPELL, {SDLK_e, KEYBIND_MOD_CTRL}, {SDLK_e, KEYBIND_MOD_CTRL}},
    {"Spell 9", "spell_9", BINDING_CATEGORY_SPELL, {SDLK_r, KEYBIND_MOD_CTRL}, {SDLK_r, KEYBIND_MOD_CTRL}},
    {"Spell 10", "spell_10", BINDING_CATEGORY_SPELL, {SDLK_t, KEYBIND_MOD_CTRL}, {SDLK_t, KEYBIND_MOD_CTRL}},

    /* Spell 11-15: Ctrl+A-G (buttons 26-30) */
    {"Spell 11", "spell_11", BINDING_CATEGORY_SPELL, {SDLK_a, KEYBIND_MOD_CTRL}, {SDLK_a, KEYBIND_MOD_CTRL}},
    {"Spell 12", "spell_12", BINDING_CATEGORY_SPELL, {SDLK_s, KEYBIND_MOD_CTRL}, {SDLK_s, KEYBIND_MOD_CTRL}},
    {"Spell 13", "spell_13", BINDING_CATEGORY_SPELL, {SDLK_d, KEYBIND_MOD_CTRL}, {SDLK_d, KEYBIND_MOD_CTRL}},
    {"Spell 14", "spell_14", BINDING_CATEGORY_SPELL, {SDLK_f, KEYBIND_MOD_CTRL}, {SDLK_f, KEYBIND_MOD_CTRL}},
    {"Spell 15", "spell_15", BINDING_CATEGORY_SPELL, {SDLK_g, KEYBIND_MOD_CTRL}, {SDLK_g, KEYBIND_MOD_CTRL}},

    /* Spell 16-20: Ctrl+Z-B (buttons 31-35) */
    {"Spell 16", "spell_16", BINDING_CATEGORY_SPELL, {SDLK_z, KEYBIND_MOD_CTRL}, {SDLK_z, KEYBIND_MOD_CTRL}},
    {"Spell 17", "spell_17", BINDING_CATEGORY_SPELL, {SDLK_x, KEYBIND_MOD_CTRL}, {SDLK_x, KEYBIND_MOD_CTRL}},
    {"Spell 18", "spell_18", BINDING_CATEGORY_SPELL, {SDLK_c, KEYBIND_MOD_CTRL}, {SDLK_c, KEYBIND_MOD_CTRL}},
    {"Spell 19", "spell_19", BINDING_CATEGORY_SPELL, {SDLK_v, KEYBIND_MOD_CTRL}, {SDLK_v, KEYBIND_MOD_CTRL}},
    {"Spell 20", "spell_20", BINDING_CATEGORY_SPELL, {SDLK_b, KEYBIND_MOD_CTRL}, {SDLK_b, KEYBIND_MOD_CTRL}},

    /* Speed Hotkeys */
    {
        "Fast Mode", SPEED_FAST_HOTKEY, BINDING_CATEGORY_SPEED, {SDLK_F1, KEYBIND_MOD_NONE},
        {SDLK_F1, KEYBIND_MOD_NONE}
    },
    {
        "Normal Mode", SPEED_NORMAL_HOTKEY, BINDING_CATEGORY_SPEED, {SDLK_F2, KEYBIND_MOD_NONE},
        {SDLK_F2, KEYBIND_MOD_NONE}
    },
    {
        "Slow Mode", SPEED_SLOW_HOTKEY, BINDING_CATEGORY_SPEED, {SDLK_F3, KEYBIND_MOD_NONE},
        {SDLK_F3, KEYBIND_MOD_NONE}
    },

    /* World Hotkeys */
    {
        "Toggle Health Percent", TOGGLE_PERCENT_HOTKEY, BINDING_CATEGORY_WORLD, {SDLK_F4, KEYBIND_MOD_NONE},
        {SDLK_F4, KEYBIND_MOD_NONE}
    },
    {
        "Toggle Stat Bases", TOGGLE_STAT_BASE_HOTKEY, BINDING_CATEGORY_WORLD, {SDLK_F5, KEYBIND_MOD_NONE},
        {SDLK_F5, KEYBIND_MOD_NONE}
    },
    {
        "Toggle Hide Sprites", TOGGLE_HIDE_SPRITE_HOTKEY, BINDING_CATEGORY_WORLD, {SDLK_F6, KEYBIND_MOD_NONE},
        {SDLK_F6, KEYBIND_MOD_NONE}
    },
    {
        "Toggle Names", TOGGLE_NAMES_HOTKEY, BINDING_CATEGORY_WORLD, {SDLK_F7, KEYBIND_MOD_NONE},
        {SDLK_F7, KEYBIND_MOD_NONE}
    },
    {
        "Toggle Health Bars", TOGGLE_HEALTH_BARS_HOTKEY, BINDING_CATEGORY_WORLD, {SDLK_F8, KEYBIND_MOD_NONE},
        {SDLK_F8, KEYBIND_MOD_NONE}
    },

    /* Window Hotkeys */
    {
        "Options Window", TOGGLE_OPTIONS_HOTKEY, BINDING_CATEGORY_WINDOW, {SDLK_F9, KEYBIND_MOD_NONE},
        {SDLK_F9, KEYBIND_MOD_NONE}
    },
    {
        "Toggle Gamma", TOGGLE_GAMMA_HOTKEY, BINDING_CATEGORY_WINDOW, {SDLK_F10, KEYBIND_MOD_NONE},
        {SDLK_F10, KEYBIND_MOD_NONE}
    },
    {
        "Reset Window Size", RESET_WINDOW_SIZE_HOTKEY, BINDING_CATEGORY_WINDOW, {SDLK_F10, KEYBIND_MOD_SHIFT},
        {SDLK_F10, KEYBIND_MOD_SHIFT}
    },

    /* General Hotkeys */
    {
        "Toggle Fightback", FIGHTBACK_HOTKEY, BINDING_CATEGORY_GENERAL, {SDLK_UNKNOWN, KEYBIND_MOD_CTRL},
        {SDLK_UNKNOWN, KEYBIND_MOD_CTRL}
    },
    {
        "Swap Gearset", SWAP_GEAR_HOTKEY, BINDING_CATEGORY_GENERAL, {SDLK_INSERT, KEYBIND_MOD_NONE},
        {SDLK_INSERT, KEYBIND_MOD_NONE}
    },
    {
        "Swap Position", SWAP_POSITION_HOTKEY, BINDING_CATEGORY_GENERAL, {SDLK_UNKNOWN, KEYBIND_MOD_CTRL},
        {SDLK_UNKNOWN, KEYBIND_MOD_CTRL}
    },
    {
        "Who's Online", WHO_HOTKEY, BINDING_CATEGORY_GENERAL, {SDLK_UNKNOWN, KEYBIND_MOD_NONE},
            {SDLK_UNKNOWN, KEYBIND_MOD_NONE}
    },
    {
        "Display Debug Info", DISPLAY_DEBUG_INFORMATION_HOTKEY, BINDING_CATEGORY_GENERAL,
        {SDLK_F11, KEYBIND_MOD_NONE}, {SDLK_F11, KEYBIND_MOD_NONE}
    },
    {"Exit", EXIT_HOTKEY, BINDING_CATEGORY_GENERAL, {SDLK_F12, KEYBIND_MOD_NONE}, {SDLK_F12, KEYBIND_MOD_NONE}},

    /* GC Hotkeys */
    {
        "Offense Mode", GC_OFFENSE_HOTKEY, BINDING_CATEGORY_GC, {SDLK_UNKNOWN, KEYBIND_MOD_NONE},
        {SDLK_UNKNOWN, KEYBIND_MOD_NONE}
    },
    {
        "Defense Mode", GC_DEFENSE_HOTKEY, BINDING_CATEGORY_GC, {SDLK_UNKNOWN, KEYBIND_MOD_NONE},
        {SDLK_UNKNOWN, KEYBIND_MOD_NONE}
    },
    {
        "Passive Mode", GC_PASSIVE_HOTKEY, BINDING_CATEGORY_GC, {SDLK_UNKNOWN, KEYBIND_MOD_NONE},
        {SDLK_UNKNOWN, KEYBIND_MOD_NONE}
    },
    {"Wait", GC_WAIT_HOTKEY, BINDING_CATEGORY_GC, {SDLK_UNKNOWN, KEYBIND_MOD_NONE}, {SDLK_UNKNOWN, KEYBIND_MOD_NONE}},
    {
        "Follow", GC_FOLLOW_HOTKEY, BINDING_CATEGORY_GC, {SDLK_UNKNOWN, KEYBIND_MOD_NONE},
        {SDLK_UNKNOWN, KEYBIND_MOD_NONE}
    },
    {"Move", GC_MOVE_HOTKEY, BINDING_CATEGORY_GC, {SDLK_UNKNOWN, KEYBIND_MOD_NONE}, {SDLK_UNKNOWN, KEYBIND_MOD_NONE}},
    {
        "Buffs", GC_BUFFS_HOTKEY, BINDING_CATEGORY_GC, {SDLK_UNKNOWN, KEYBIND_MOD_NONE},
        {SDLK_UNKNOWN, KEYBIND_MOD_NONE}
    },
    {
        "Transfer", GC_TRANSFER_HOTKEY, BINDING_CATEGORY_GC, {SDLK_UNKNOWN, KEYBIND_MOD_NONE},
        {SDLK_UNKNOWN, KEYBIND_MOD_NONE}
    }
};

/* Initialize Global config and ensure defaults are set */
void keybindings_init(void) {
    // Only initialize the registry once
    if (g_config.keybind.num_bindings == 0) {
        g_config.keybind.bindings = binding_registry;
        g_config.keybind.num_bindings = sizeof(binding_registry) / sizeof(binding_registry[0]);
    }
    for (int i = 0; i < g_config.keybind.num_bindings; i++) {
        g_config.keybind.bindings[i].keybinding = binding_registry[i].default_keybinding;
    }
}

/* Helper function to get key name */
static const char *get_key_name(SDL_Keycode key) {
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

const char *keybinding_to_short_string(Keybinding kb, char *buffer, int buffer_size) {
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
    const char *key_name = get_key_name(kb.key);

    /* For single letter keys, show uppercase */
    if (kb.key >= SDLK_a && kb.key <= SDLK_z) {
        char key_char[2];
        key_char[0] = (char) (kb.key - SDLK_a + 'A');
        key_char[1] = '\0';
        snprintf(buffer, buffer_size, "%s%s", mod_str, key_char);
    } else if (kb.key >= SDLK_1 && kb.key <= SDLK_9) {
        /* Number keys 1-9 */
        char key_char[2];
        key_char[0] = (char) (kb.key - SDLK_1 + '1');
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
const char *keybinding_to_string(Keybinding kb, char *buffer, int buffer_size) {
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
    const char *key_name = get_key_name(kb.key);

    /* For single letter keys, show uppercase */
    if (kb.key >= SDLK_a && kb.key <= SDLK_z) {
        char key_char[2];
        key_char[0] = (char) (kb.key - SDLK_a + 'A');
        key_char[1] = '\0';
        snprintf(buffer, buffer_size, "%s%s", mod_str, key_char);
    } else if (kb.key >= SDLK_1 && kb.key <= SDLK_9) {
        /* Number keys 1-9 */
        char key_char[2];
        key_char[0] = (char) (kb.key - SDLK_1 + '1');
        key_char[1] = '\0';
        snprintf(buffer, buffer_size, "%s%s", mod_str, key_char);
    } else if (kb.key == SDLK_0) {
        snprintf(buffer, buffer_size, "%s0", mod_str);
    } else {
        snprintf(buffer, buffer_size, "%s%s", mod_str, key_name);
    }

    return buffer;
}

BindingDescriptor *binding_find(SDL_Keycode key, int sdl_modstate) {
    bool has_shift = (sdl_modstate & KMOD_SHIFT) != 0;
    bool has_ctrl = (sdl_modstate & KMOD_CTRL) != 0;
    bool has_alt = (sdl_modstate & KMOD_ALT) != 0;

    /* Build current modifier state */
    KeybindModifier current_mod = KEYBIND_MOD_NONE;
    if (has_shift) current_mod |= KEYBIND_MOD_SHIFT;
    if (has_ctrl) current_mod |= KEYBIND_MOD_CTRL;
    if (has_alt) current_mod |= KEYBIND_MOD_ALT;

    for (int i = 0; i < g_config.keybind.num_bindings; i++) {
        if (g_config.keybind.bindings[i].keybinding.key == key &&
            g_config.keybind.bindings[i].keybinding.modifier == current_mod) {
            return &g_config.keybind.bindings[i];
        }
    }

    return NULL; /* No match found */
}

BindingDescriptor *binding_find_by_id(const char *id) {
    for (int i = 0; i < g_config.keybind.num_bindings; i++) {
        if (strcmp(g_config.keybind.bindings[i].id, id) == 0) {
            return &g_config.keybind.bindings[i];
        }
    }

    return NULL; /* No match found */
}
