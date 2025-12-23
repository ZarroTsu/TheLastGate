#include "config.h"

#include <SDL2/SDL_filesystem.h>
#include <string.h>

#include "main.h"
#include "graphics/sdl.h"
#include "game/game_input.h"
#include "log/log.h"

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
        .base_path = "",
        .pref_path = "",
        .show_performance = false
    }
};

void apply_config_changes(void) {
    if (g_game_state == GAME_STATE_GAME) {
        sync_chat_input_state();
    }
}

void config_init_paths(void) {
    /* Initialize preference path (SDL manages this memory) */
    char *sdl_pref_path = SDL_GetPrefPath("TheLastGate", "TheLastGate");
    if (sdl_pref_path) {
        strncpy(g_config.runtime.pref_path, sdl_pref_path, sizeof(g_config.runtime.pref_path) - 1);
        g_config.runtime.pref_path[sizeof(g_config.runtime.pref_path) - 1] = '\0';
        SDL_free(sdl_pref_path);
        log_info("Preferences directory: %s", g_config.runtime.pref_path);
    } else {
        log_error("Failed to get preferences path: %s", SDL_GetError());
        g_config.runtime.pref_path[0] = '\0';
    }

    /* Initialize base path (game directory) */
    char *sdl_base_path = SDL_GetBasePath();
    if (sdl_base_path) {
        strncpy(g_config.runtime.base_path, sdl_base_path, sizeof(g_config.runtime.base_path) - 1);
        g_config.runtime.base_path[sizeof(g_config.runtime.base_path) - 1] = '\0';
        SDL_free(sdl_base_path);
        log_info("Base directory: %s", g_config.runtime.base_path);
    } else {
        log_error("Failed to get base path: %s", SDL_GetError());
        g_config.runtime.base_path[0] = '\0';
    }
}

void config_cleanup_paths(void) {
    /* Paths are stored in our own buffers, so no cleanup needed */
    /* This function exists for API consistency and future extensibility */
}
