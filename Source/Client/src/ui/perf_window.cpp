//
// Created by james on 12/23/2025.
//

#include "perf_window.hpp"

#include <imgui.h>

#include "log/log.h"

void PerfWindow::Render(const PerfWindowState *state) {
    if (ImGui::Begin("Performance")) {
        RenderStats(state);
    }
    ImGui::End();
}

void PerfWindow::RenderStats(const PerfWindowState *state) {
    ImGui::Text("FPS: %.0ffps", state->fps_avg);
    ImGui::SameLine();
    ImGui::Text("Fastest: %fms", state->sample_ms_min);
    ImGui::SameLine();
    ImGui::Text("Slowest: %fms", state->sample_ms_max);
}

void perf_window_render(const PerfWindowState *state) {
    PerfWindow::Render(state);
}
