//
// Created by james on 12/6/2025.
//

#include "give_more.h"
#include "engine.h"
#include "common.h"
#include "inter.h"

static LastGiven last_given = {
    .sprite_id = 0,
    .mod_flags = 0,
    .stack_size = 0
};

void set_last_given(int sprite_id, int mod_flags, char stack_size) {
    last_given.sprite_id = sprite_id;
    last_given.mod_flags = mod_flags;
    last_given.stack_size = stack_size;
}

void cursor_emptied(void) {
    if (last_given.sprite_id == 0) return;

    for (int i = 0; i < MAXITEMS; i++) {
        if (pl.item[i] == last_given.sprite_id && pl.item_p[i] == last_given.mod_flags) {
            cmd3(CL_CMD_INV, 0, i, 0);
            break;
        }
    }

    last_given.sprite_id = 0;
    last_given.mod_flags = 0;
    last_given.stack_size = 0;
}
