//
// Created by james on 12/15/2025.
//

#include "ui.h"

#include <imgui_freetype.h>

#include "hotbar.hpp"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl2.h"
#include "inventory.hpp"
#include "launcher_ui.hpp"
#include "option_window.hpp"
#include "perf_window.hpp"
#include "config/config.h"
#include "game/game_ui.h"
#include "graphics/sdl.h"

static void start_frame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();

    ImGuiIO &io = ImGui::GetIO();
    io.DisplaySize = ImVec2(1280, 720);
    io.DisplayFramebufferScale = ImVec2(1, 1);
    ImGui::NewFrame();
}

static void render() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

static ImFont *load_font(const char *filename, float size_pixels) {
    ImGuiIO &io = ImGui::GetIO();

    io.Fonts->Flags = ImFontAtlasFlags_NoBakedLines;
    /* Configure for pixel-perfect rendering */
    ImFontConfig config;
    config.OversampleH = 2; /* No horizontal oversampling */
    config.OversampleV = 1; /* No vertical oversampling */
    config.PixelSnapH = true; /* Align to pixel grid */
    config.PixelSnapV = true;
    config.RasterizerMultiply = 1.0F;
    config.RasterizerDensity = 1.0F;
    config.FontLoaderFlags = ImGuiFreeTypeBuilderFlags_Bitmap;

    return io.Fonts->AddFontFromFileTTF(filename, size_pixels, &config);
}

static void load_fonts() {
    font_sizes.ui = load_font("resources/astonia.ttf", 10);
    font_sizes.normal = load_font("resources/PixelOperator.ttf", 16);
    font_sizes.large = load_font("resources/PixelOperator.ttf", 24);
    font_sizes.subheader = load_font("resources/PixelOperator.ttf", 28);
    font_sizes.header = load_font("resources/PixelOperator.ttf", 32);

    font_sizes_bold.ui = load_font("resources/astonia.ttf", 10);
    font_sizes_bold.normal = load_font("resources/PixelOperator-Bold.ttf", 16);
    font_sizes_bold.large = load_font("resources/PixelOperator-Bold.ttf", 24);
    font_sizes_bold.subheader = load_font("resources/PixelOperator-Bold.ttf", 28);
    font_sizes_bold.header = load_font("resources/PixelOperator-Bold.ttf", 32);

    ImGuiIO &io = ImGui::GetIO();
    io.FontDefault = static_cast<ImFont *>(font_sizes.ui);
}


EXTERN_C_BEGIN
#include <stddef.h>

FontSizes font_sizes = {0};
FontSizes font_sizes_bold = {0};

void ui_init(void *sdl_window, void *gl_context) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();

    io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;
    io.FontGlobalScale = 1.0F;

    /* Setup Dear ImGui style */
    ImGui::StyleColorsDark();

    /* Setup platform/renderer backends */
    ImGui_ImplSDL2_InitForOpenGL(static_cast<SDL_Window *>(sdl_window), gl_context);
    ImGui_ImplOpenGL3_Init("#version 410 core");

    io.DisplaySize = ImVec2(SCREEN_WIDTH, SCREEN_HEIGHT);
    io.DisplayFramebufferScale = ImVec2(1, 1);

    load_fonts();
}

void ui_shutdown(void) {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void ui_render(const UIState ui_state) {
    start_frame();

    if (ui_state == UI_STATE_LAUNCHER) {
        launcher_ui_render();
    } else {
        spell_hud();

        if (game_ui_state.show_options) {
            options_window_render();
        }

        inventory_render();
        quick_stats_render(&game_ui_state.quick_stat_state);
        if (g_config.runtime.show_performance) {
            perf_window_render(&game_ui_state.perf_window_state);
        }
    }

    render();
}

EXTERN_C_END
