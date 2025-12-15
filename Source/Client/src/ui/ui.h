#pragma once

typedef struct {
    void *normal;
    void *large;
    void *subheader;
    void *header;
} FontSizes;

extern FontSizes font_sizes;