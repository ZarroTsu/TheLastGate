#pragma once

#define UI_CENTER_X (SCREEN_WIDTH / 2)
#define UI_CENTER_Y (SCREEN_HEIGHT / 2)

typedef struct {
    void *ui;
    void *normal;
    void *large;
    void *subheader;
    void *header;
} FontSizes;

extern FontSizes font_sizes;
extern FontSizes font_sizes_bold;