#pragma once
#include <stdbool.h>

extern bool is_resetting_keybinds;

void options_window_render(void);
void options_window_scroll(int x, int y, int delta);