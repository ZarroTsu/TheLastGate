#pragma once

#include <SDL2/SDL.h>

#define NUM_SPELL_HOTKEYS 20

/* Modifier flags - match SDL but use simpler names */
typedef enum {
    KEYBIND_MOD_NONE = 0,
    KEYBIND_MOD_CTRL = 1,
    KEYBIND_MOD_ALT = 2
} KeybindModifier;

/* Represents a single keybinding */
typedef struct {
    SDL_Keycode key;           /* SDL_Keycode (e.g., SDLK_1, SDLK_a) */
    KeybindModifier modifier;  /* CTRL or ALT only */
} Keybinding;

/* Global keybinding configuration */
typedef struct {
    Keybinding spell_hotkeys[NUM_SPELL_HOTKEYS];
} KeybindConfig;

/* Global instance - defined in keybindings.c */
extern KeybindConfig keybind_config;

/* Initialization - call once at startup */
void keybindings_init(void);

/* Utility function to convert keybinding to display string like "Ctrl+A" */
const char* keybinding_to_short_string(Keybinding kb, char* buffer, int buffer_size);
const char* keybinding_to_string(Keybinding kb, char* buffer, int buffer_size);
