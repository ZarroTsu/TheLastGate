#pragma once
#include <stdbool.h>

bool security_acquire_lock(const char *path);
void security_release_lock();
