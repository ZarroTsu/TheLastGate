#ifndef THELASTGATE_LOG_H
#define THELASTGATE_LOG_H


#pragma once

#ifdef NDEBUG
#define LOG(...) ((void)0)
#else
#define LOG(...) printf(__VA_ARGS__)
#endif

#endif //THELASTGATE_LOG_H
