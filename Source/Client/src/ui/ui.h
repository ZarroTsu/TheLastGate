#pragma once

#include "common.h"

EXTERN_C_BEGIN

#define UI_CENTER_X (SCREEN_WIDTH / 2)
#define UI_CENTER_Y (SCREEN_HEIGHT / 2)

typedef enum {
    UI_STATE_LAUNCHER,
    UI_STATE_GAME
} UIState;

typedef struct {
    void *ui;
    void *normal;
    void *large;
    void *subheader;
    void *header;
} FontSizes;

extern FontSizes font_sizes;
extern FontSizes font_sizes_bold;

void ui_init(void *sdl_window, void *gl_context);
void ui_shutdown(void);
void ui_render(UIState ui_state);

EXTERN_C_END