#pragma once


#pragma once

#ifdef NDEBUG
#define LOG(...) ((void)0)
#else
#define LOG(...) printf(__VA_ARGS__)
#endif
