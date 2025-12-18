#include "security.h"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static HANDLE lock_handle = NULL;

bool security_acquire_lock(const char *path) {
    lock_handle = CreateFileA(path,
                              GENERIC_READ | GENERIC_WRITE,
                              0,
                              NULL,
                              OPEN_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL,
                              NULL
    );

    return lock_handle != INVALID_HANDLE_VALUE;
}

void security_release_lock() {
    if (lock_handle) {
        CloseHandle(lock_handle);
        lock_handle = NULL;
    }
}
#else

#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>

static int lock_fd = -1;

bool security_acquire_lock(const char *path) {
    lock_fd = open(path, O_CREAT | O_RDWR, 0666);
    if (lock_fd < 0) return false;

    return flock(lock_fd, LOCK_EX | LOCK_NB) == 0;
}

void security_release_lock() {
    if (lock_fd >= 0) {
        close(lock_fd);
        lock_fd = -1;
    }
}

#endif

