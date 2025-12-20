#pragma once

#include <stdbool.h>
#include <SDL2/SDL.h>

typedef struct {
    int major, minor;
} SdlClientVersion;

const extern SdlClientVersion CLIENT_VERSION;
extern int quit;
extern int cur_pos;
extern int in_len;
extern int view_pos;
extern int tabmode;
extern int tabstart;
extern int chat_mode_active;
extern int logstart;
extern int logtimer;
extern int mx, my;

extern int do_alpha;
extern int do_shadow;
extern int do_darkmode;

extern char input[128];
extern short screen_renderdist;

// Chat History
extern char history[20][128];
extern int hist_nr;
extern int hist_len[20];


extern SDL_Cursor *cursors[10];

void complete_word(void);

void add_words(void);

