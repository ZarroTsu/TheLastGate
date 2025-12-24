#pragma once

#define PERFORMANCE_HISTORY 60

typedef struct {
    float sample_ms_last;
    float sample_ms_min;
    float sample_ms_max;
    float fps_avg;
} PerfStats;

void perf_init(void);
void perf_begin(void);
void perf_end(void);
void perf_commit(void);
const PerfStats* perf_get_stats(void);