#pragma once

#include <SDL2/SDL.h>

#define NUM_SPELL_HOTKEYS 20
#define NUM_GENERAL_HOTKEYS 1

// GENERAL HOTKEYS
#define FIGHTBACK_HOTKEY 0
//SWAP

// GC Geronimo
// GC Offense
// GC Defense
// GC Passive
// GC wait
// gc follow
// gc move
// Recall GC

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
    Keybinding general_hotkeys[NUM_GENERAL_HOTKEYS];
} KeybindConfig;

/* Global instance - defined in keybindings.c */
extern KeybindConfig keybind_config;

/* Initialization - call once at startup */
void keybindings_init(void);

/* Utility function to convert keybinding to display string like "Ctrl+A" */
const char* keybinding_to_short_string(Keybinding kb, char* buffer, int buffer_size);
const char* keybinding_to_string(Keybinding kb, char* buffer, int buffer_size);

/* Find spell slot (0-19) matching key+modifier, returns -1 if no match */
int keybinding_find_spell_slot(SDL_Keycode key, int sdl_modstate);

/* Check other non-spell keybinds, returns the toggle id or -1 if no match */
int keybinding_find_general(SDL_KeyCode key, int sdl_modstate);