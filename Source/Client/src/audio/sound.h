#pragma once

extern int do_music;
extern int do_sound;

int init_sound(void);

int play_sound(const char *file, int vol, int p);

void cleanup_sound(void);
