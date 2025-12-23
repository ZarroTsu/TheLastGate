#include "perf.h"

#include <SDL_timer.h>
#include <stdint.h>

static uint64_t segment_start = 0;
static double freq_inv = 0.0;

static double accumulated_ms = 0.0;
static int segment_active = 0;

static float samples[PERFORMANCE_HISTORY];
static int sample_index;
static int sample_count;

static PerfStats stats;

void perf_init(void) {
    freq_inv = 1000.0 / (double) SDL_GetPerformanceFrequency();

    for (int i = 0; i < PERFORMANCE_HISTORY; i++) {
        samples[i] = 0.0f;
    }

    accumulated_ms = 0.0;
    segment_active = 0;
    sample_index = 0;
    sample_count = 0;

    stats.sample_ms_last = 0.0f;
    stats.sample_ms_max = 0.0f;
    stats.sample_ms_min = 0.0f;
    stats.fps_avg = 0.0f;
}

void perf_begin(void) {
    if (!segment_active) {
        segment_start = SDL_GetPerformanceCounter();
        segment_active = 1;
    }

}

void perf_end(void) {
    if (!segment_active) return;

    uint64_t end = SDL_GetPerformanceCounter();
    uint64_t delta = end - segment_start;

    accumulated_ms += (double)delta * freq_inv;
    segment_active = 0;
}

void perf_commit(void) {

    if (segment_active) perf_end();

    float ms = (float)accumulated_ms;
    accumulated_ms = 0.0f;

    stats.sample_ms_last = ms;

    samples[sample_index] = ms;
    sample_index = (sample_index + 1) % PERFORMANCE_HISTORY;
    if (sample_count < PERFORMANCE_HISTORY) {
        sample_count++;
    }

    float min_ms = FLT_MAX;
    float max_ms = 0.0f;
    float sum_ms = 0.0f;

    for (int i = 0; i < sample_count; i++) {
        float t = samples[i];
        if (t < min_ms) min_ms = t;
        if (t > max_ms) max_ms = t;
        sum_ms += t;
    }

    stats.sample_ms_min = (sample_count > 0) ? min_ms : 0.0f;
    stats.sample_ms_max = max_ms;

    if (sum_ms > 0.0f) {
        float avg_ms = sum_ms / (float)sample_count;
        stats.fps_avg = 1000.0 / avg_ms;
    } else {
        stats.fps_avg = 0.0f;
    }
}


const PerfStats *perf_get_stats(void) {
    return &stats;
}
