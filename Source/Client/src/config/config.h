#pragma once
#include <stdbool.h>


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
} UiConfig;


typedef struct {
    bool give_more;
    bool use_queue;
} GameConfig;


typedef struct {
    char path[256];
} RuntimeConfig;

typedef struct {
    VideoConfig video;
    AudioConfig audio;
    UiConfig ui;
    GameConfig gameplay;
    RuntimeConfig runtime;
} GlobalConfig;

extern GlobalConfig g_config;