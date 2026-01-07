#include "launcher.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "inter.h"
#include "main.h"
#include "character_manager.h"
#include "graphics/sdl.h"
#include "log/log.h"
#include "net/connection.h"
#include "net/socket.h"

LauncherState launcher_state = {0};

void launcher_init() {
    okey_to_class_gender(&launcher_state.current_class, &launcher_state.current_gender);
    init_previous_characters();
}

void launcher_shutdown() {
    save_previous_characters();
}

void launcher_tick() {
    launcher_state.connection_status = connection_update();
    if (launcher_state.is_connecting) {
        snprintf(launcher_state.connection_status_text, sizeof(launcher_state.connection_status_text), "STATUS: %s",
                 launcher_state.connection_status.status_message);
        if (launcher_state.connection_status.state == CONNECTION_STATE_CONNECTED) {
            launcher_state.is_connecting = false;
            save_options();
            g_game_state = GAME_STATE_GAME;
        } else if (launcher_state.connection_status.state == CONNECTION_STATE_ERROR) {
            launcher_state.is_connecting = 0;
            log_error("Connection failed: %s", launcher_state.connection_status.error_message);
            snprintf(launcher_state.connection_status_text, sizeof(launcher_state.connection_status_text), "STATUS: %s",
                     launcher_state.connection_status.error_message);
        }
    }
}

void launcher_connect(void) {
    const int race = class_gender_to_okey(launcher_state.current_class, launcher_state.current_gender);
    const int sex = launcher_state.current_gender + 1; /* Convert 0/1 to 1/2 */

    strncpy(passwd, launcher_state.password, sizeof(passwd) - 1);
    passwd[sizeof(passwd) - 1] = '\0';

    save_options();
    connection_init();
    connection_start(race, sex);
    launcher_state.is_connecting = 1;
}

void launcher_set_fyi(const char *text) {
    snprintf(launcher_state.simple_popover_state.text, sizeof(launcher_state.simple_popover_state.text), "%s", text);
    launcher_state.simple_popover_state.display = true;
}
