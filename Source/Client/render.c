#include <ddraw.h>

#include "DD.H"
#include "SDL.h"

int init_windowed(HWND hwnd, int x, int y) {
    if (sdl_init()) {
        return -1;
    }
    if (dd_init_windowed(hwnd, x, y)) {
        return -1;
    };
    return 0;
}

int init(HWND hwnd, int x, int y) {
    if (sdl_init()) {
        return -1;
    }
    if (dd_init(hwnd, x, y)) {
        return -1;
    };
    return 0;
}
