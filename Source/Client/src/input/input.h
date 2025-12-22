#pragma once
#include <SDL_events.h>
#include <stdbool.h>

extern int quit;
extern SDL_Event input_events[128];
extern int input_event_count;
extern bool window_resetting;

void input_init(void);
void handle_input(void);