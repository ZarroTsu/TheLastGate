#pragma once

typedef struct {
    void *normal;
    void *large;
    void *subheader;
    void *header;
    void *big_header;
} FontSizes;

extern FontSizes font_sizes;