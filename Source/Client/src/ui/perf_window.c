//
// Created by james on 12/23/2025.
//

#include "perf_window.h"

#include <stddef.h>
#include <stdio.h>

#include "imgui/imgui_wrapper.h"
#include "util/perf.h"

void render_perf_window(void) {
    if (imgui_begin("Performance", NULL, 0)) {
        char fps[32];
        char fastest_ms[32];
        char slowest_ms[32];
        snprintf(fps, sizeof(fps), "%.0ffps", perf_get_stats()->fps_avg);
        snprintf(fastest_ms, sizeof(fastest_ms), "%fms", perf_get_stats()->sample_ms_min);
        snprintf(slowest_ms, sizeof(slowest_ms), "%fms", perf_get_stats()->sample_ms_max);
        imgui_text("FPS:"); imgui_same_line_gap(); imgui_text(fps);
        imgui_text("Fastest:"); imgui_same_line_gap(); imgui_text(fastest_ms);
        imgui_text("Slowest:"); imgui_same_line_gap(); imgui_text(slowest_ms);
    }
    imgui_end();
}
