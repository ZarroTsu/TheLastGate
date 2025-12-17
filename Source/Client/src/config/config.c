#include "config.h"

#include "graphics/sdl.h"

GlobalConfig g_config = {
    .video = {
        .gamma = 5000.0f,
        .windowed = true,
        .window_size = {SCREEN_WIDTH, SCREEN_HEIGHT}
    },
    .audio = {
        .music_enabled = true,
        .music_volume = 10,
        .sound_enabled = true,
        .sound_volume = 10
    },
    .ui = {
        .cost_helper = false,
        .escape_closes_menu_first = true,
        .enter_to_talk = false
    },
    .gameplay = {
        .give_more = false,
        .use_queue = false
    },
    .runtime = {
        .path = ""
    }
};
