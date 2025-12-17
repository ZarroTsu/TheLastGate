#pragma once

#include <SDL2/SDL.h>

#define NUM_SPELL_HOTKEYS 20
#define NUM_GENERAL_HOTKEYS 23

// SPEED HOTKEYS
#define SPEED_SLOW_HOTKEY 10
#define SPEED_NORMAL_HOTKEY 11
#define SPEED_FAST_HOTKEY 12

// WORLD HOTKEYS
#define TOGGLE_PERCENT_HOTKEY 13
#define TOGGLE_STAT_BASE_HOTKEY 14
#define TOGGLE_HIDE_SPRITE_HOTKEY 15
#define TOGGLE_NAMES_HOTKEY 16
#define TOGGLE_HEALTH_BARS_HOTKEY 17

// WINDOW HOTKEYS
#define TOGGLE_OPTIONS_HOTKEY 18
#define TOGGLE_GAMMA_HOTKEY 19
#define RESET_WINDOW_SIZE_HOTKEY 20

// GENERAL HOTKEYS
#define FIGHTBACK_HOTKEY 0
#define SWAP_POSITION_HOTKEY 1
#define SWAP_GEAR_HOTKEY 9
#define DISPLAY_DEBUG_INFORMATION_HOTKEY 21
#define EXIT_HOTKEY 22

// GC HOTKEYS
#define GC_OFFENSE_HOTKEY 2
#define GC_DEFENSE_HOTKEY 3
#define GC_PASSIVE_HOTKEY 4
#define GC_WAIT_HOTKEY 5
#define GC_FOLLOW_HOTKEY 6
#define GC_MOVE_HOTKEY 7
#define GC_BUFFS_HOTKEY 8

/* Used for organization in UI */
typedef enum {
    BINDING_CATEGORY_SPELL,
    BINDING_CATEGORY_SPEED,
    BINDING_CATEGORY_WORLD,
    BINDING_CATEGORY_WINDOW,
    BINDING_CATEGORY_GENERAL,
    BINDING_CATEGORY_GC
} BindingCategory;

/* Modifier flags - support any combination of modifiers */
typedef enum {
    KEYBIND_MOD_NONE = 0,
    KEYBIND_MOD_SHIFT = 1,
    KEYBIND_MOD_CTRL = 2,
    KEYBIND_MOD_ALT = 4,
    KEYBIND_MOD_SHIFT_CTRL = 3,    /* SHIFT + CTRL */
    KEYBIND_MOD_SHIFT_ALT = 5,     /* SHIFT + ALT */
    KEYBIND_MOD_CTRL_ALT = 6,      /* CTRL + ALT */
    KEYBIND_MOD_SHIFT_CTRL_ALT = 7 /* SHIFT + CTRL + ALT */
} KeybindModifier;

/* Represents a single keybinding */
typedef struct {
    SDL_Keycode key;           /* SDL_Keycode (e.g., SDLK_1, SDLK_a) */
    KeybindModifier modifier;  /* CTRL or ALT only */
} Keybinding;

/* Describes a single bindable action */
typedef struct {
    const char* name;
    const char* id;
    BindingCategory category;
    Keybinding default_keybinding;
    Keybinding keybinding;
} BindingDescriptor;

/* Global keybinding configuration */
typedef struct {
    BindingDescriptor *bindings;
    int num_bindings;
} KeybindConfig;

/* Initialization - call once at startup */
void keybindings_init(void);

/* Utility function to convert keybinding to display string like "Ctrl+A" */
const char* keybinding_to_short_string(Keybinding kb, char* buffer, int buffer_size);
const char* keybinding_to_string(Keybinding kb, char* buffer, int buffer_size);

/* Find binding by current binding, returns NULL otherwise */
BindingDescriptor *binding_find(SDL_Keycode key, int sdl_modstate);

/* Find binding by ID, returns NULL otherwise */
BindingDescriptor *binding_find_by_id(const char* id);