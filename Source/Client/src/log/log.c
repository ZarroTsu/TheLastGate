#include "log.h"

typedef struct {
    FILE *info;
    FILE *warn;
    FILE *error;
} LogFiles;

static const char *sdl_log_category_name(const int category) {
    switch (category) {
        case SDL_LOG_CATEGORY_APPLICATION: return "APPLICATION";
        default: return "DEFAULT";
    }
}

static void log_callback(
    void *userdata,
    const int category,
    const SDL_LogPriority priority,
    const char *message) {
    const LogFiles *logs = userdata;
    FILE *out = NULL;

    switch (priority) {
        case SDL_LOG_PRIORITY_VERBOSE:
        case SDL_LOG_PRIORITY_DEBUG:
        case SDL_LOG_PRIORITY_INFO:
            out = logs->info;
            break;
        case SDL_LOG_PRIORITY_WARN:
            out = logs->warn;
            break;
        case SDL_LOG_PRIORITY_ERROR:
        case SDL_LOG_PRIORITY_CRITICAL:
            out = logs->error;
            break;
        default:
            out = logs->info;
            break;
    }

    if (out) {
        fprintf(out, "[%s] [%s]\n", sdl_log_category_name(category), message);
        fflush(out);
    }

    fprintf(stderr, "[%s] %s\n", sdl_log_category_name(category), message);
}

static LogFiles logs = {0};

void log_init() {
    logs.info = fopen("info.log", "w");
    logs.warn = fopen("warn.log", "w");
    logs.error = fopen("error.log", "w");
    SDL_LogSetOutputFunction(log_callback, &logs);
}

void log_shutdown() {
    fclose(logs.info);
    fclose(logs.warn);
    fclose(logs.error);
    SDL_LogSetOutputFunction(NULL, NULL);
}
