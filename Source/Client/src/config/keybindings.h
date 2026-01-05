#pragma once

#include "common.h"
EXTERN_C_BEGIN
#include <SDL2/SDL.h>

#define NUM_SPELL_HOTKEYS 20
#define NUM_GENERAL_HOTKEYS 26

// SPEED HOTKEYS
#define SPEED_SLOW_HOTKEY "speed_slow_hotkey"
#define SPEED_NORMAL_HOTKEY "speed_normal_hotkey"
#define SPEED_FAST_HOTKEY "speed_fast_hotkey"

// WORLD HOTKEYS
#define TOGGLE_PERCENT_HOTKEY "toggle_percent_hotkey"
#define TOGGLE_STAT_BASE_HOTKEY "toggle_stat_base_hotkey"
#define TOGGLE_HIDE_SPRITE_HOTKEY "toggle_hide_sprite_hotkey"
#define TOGGLE_NAMES_HOTKEY "toggle_names_hotkey"
#define TOGGLE_HEALTH_BARS_HOTKEY "toggle_health_bars_hotkey"

// WINDOW HOTKEYS
#define TOGGLE_OPTIONS_HOTKEY "toggle_options_hotkey"
#define TOGGLE_GAMMA_HOTKEY "toggle_gamma_hotkey"
#define RESET_WINDOW_SIZE_HOTKEY "reset_window_size_hotkey"

// GENERAL HOTKEYS
#define FIGHTBACK_HOTKEY "fightback_hotkey"
#define SWAP_POSITION_HOTKEY "swap_position_hotkey"
#define SWAP_GEAR_HOTKEY "swap_gear_hotkey"
#define WHO_HOTKEY "who_hotkey"
#define DISPLAY_DEBUG_INFORMATION_HOTKEY "display_debug_information_hotkey"
#define EXIT_HOTKEY "exit_hotkey"
#define MY_BUFF_HOTKEY "my_buff_hotkey"

// GC HOTKEYS
#define GC_OFFENSE_HOTKEY "gc_offense_hotkey"
#define GC_DEFENSE_HOTKEY "gc_defense_hotkey"
#define GC_PASSIVE_HOTKEY "gc_passive_hotkey"
#define GC_WAIT_HOTKEY "gc_wait_hotkey"
#define GC_FOLLOW_HOTKEY "gc_follow_hotkey"
#define GC_MOVE_HOTKEY "gc_move_hotkey"
#define GC_BUFFS_HOTKEY "gc_buffs_hotkey"
#define GC_TRANSFER_HOTKEY "gc_transfer_hotkey"

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
EXTERN_C_END