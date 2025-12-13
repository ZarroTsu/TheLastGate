#pragma once
#include <SDL2/SDL.h>

#define LOG_CATEGORY_DEFAULT SDL_LOG_CATEGORY_APPLICATION

#define log_critical(fmt, ...) \
SDL_LogMessage(LOG_CATEGORY_DEFAULT, SDL_LOG_PRIORITY_CRITICAL, fmt, ##__VA_ARGS__)

#define log_error(fmt, ...) \
SDL_LogMessage(LOG_CATEGORY_DEFAULT, SDL_LOG_PRIORITY_ERROR, fmt, ##__VA_ARGS__)

#define log_warning(fmt, ...) \
SDL_LogMessage(LOG_CATEGORY_DEFAULT, SDL_LOG_PRIORITY_WARN, fmt, ##__VA_ARGS__)

#define log_info(fmt, ...) \
SDL_LogMessage(LOG_CATEGORY_DEFAULT, SDL_LOG_PRIORITY_INFO, fmt, ##__VA_ARGS__)

#ifdef NDEBUG
#define log_debug(...) ((void)0)
#else
#define log_debug(fmt, ...) \
SDL_LogMessage(LOG_CATEGORY_DEFAULT, SDL_LOG_PRIORITY_DEBUG, fmt, ##__VA_ARGS__)
#endif


void log_init();
void log_shutdown();