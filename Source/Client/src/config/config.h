#pragma once
#include <stdbool.h>
#include "config/keybindings.h"


typedef struct {
    float gamma;
    bool windowed;
    int window_size[2];
} VideoConfig;


typedef struct {
    bool sound_enabled;
    int sound_volume;
    bool music_enabled;
    int music_volume;
} AudioConfig;


typedef struct {
    bool escape_closes_menu_first;
    bool cost_helper;
    bool enter_to_talk;
} UiConfig;


typedef struct {
    bool give_more;
    bool use_queue;
    bool stubborn_use;
    bool stubborn_move;
    bool stubborn_give;
    bool stubborn_pickup;
    bool stubborn_drop;
} GameConfig;


typedef struct {
    char base_path[256];      // Game directory
    char pref_path[256];      // SDL preference path for saves/config
    bool show_performance;
} RuntimeConfig;

typedef struct {
    VideoConfig video;
    AudioConfig audio;
    UiConfig ui;
    GameConfig gameplay;
    RuntimeConfig runtime;
    KeybindConfig keybind;
} GlobalConfig;

extern GlobalConfig g_config;

/* Initialize runtime paths (pref_path, base_path) */
void config_init_paths(void);

/* Clean up runtime paths */
void config_cleanup_paths(void);

/* Apply config changes that require runtime state synchronization */
void apply_config_changes(void);