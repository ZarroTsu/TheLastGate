#pragma once

typedef struct {
    int sprite_id;
    int mod_flags;
    char stack_size;
} LastGiven;

void set_last_given(int sprite_id, int mod_flags, char stack_size);
void cursor_emptied(void);