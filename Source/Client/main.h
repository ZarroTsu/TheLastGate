#pragma once

#include <stdbool.h>
#include <SDL2/SDL.h>

typedef struct {
    int tricky_flag;
} AppState;

typedef struct {
    int major, minor;
} SdlClientVersion;

const extern SdlClientVersion CLIENT_VERSION;
extern AppState app_state;
extern int quit;
extern int cur_pos;
extern int in_len;
extern int view_pos;
extern int tabmode;
extern int tabstart;
extern int logstart;
extern int logtimer;
extern int mx, my;

extern char input[128];

// Chat History
extern char history[20][128];
extern int hist_nr;
extern int hist_len[20];

extern SDL_Cursor *cursors[10];

void complete_word(void);

void add_words(void);

