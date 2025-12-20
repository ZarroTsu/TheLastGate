#pragma once

typedef struct {
    void *normal;
    void *large;
    void *subheader;
    void *header;
    void *big_header;
} FontSizes;

extern void *font_pixel_times;
extern FontSizes font_sizes;
extern FontSizes font_sizes_bold;