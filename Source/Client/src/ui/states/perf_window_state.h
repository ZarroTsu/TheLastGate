#pragma once
#include "common.h"

EXTERN_C_BEGIN

typedef struct {
    float fps_avg;
    float sample_ms_min;
    float sample_ms_max;
} PerfWindowState;

EXTERN_C_END
