#pragma once

int sound_init(void);

int play_sound(const char *file, int vol, int p);

void sound_shutdown(void);
