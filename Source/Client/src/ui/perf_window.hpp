#pragma once
#include "states/perf_window_state.h"

class PerfWindow {
public:
    static void Render(const PerfWindowState *state);

private:
    static void RenderStats(const PerfWindowState *state);
};

void perf_window_render(const PerfWindowState *state);