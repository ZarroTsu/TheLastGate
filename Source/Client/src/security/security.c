#include "security.h"

#include <stdint.h>

#include "config/config.h"
#include "log/log.h"

#if defined(_WIN32)
#include <windows.h>
#include <bcrypt.h>
#else
#include <sys/random.h>
#endif


static uint32_t get_seed() {
    uint32_t seed;

#if defined(_WIN32)
    BCryptGenRandom(NULL, (PUCHAR) &seed, sizeof(seed), BCRYPT_USE_SYSTEM_PREFERRED_RNG);
#else
    getrandom(&seed, sizeof(seed), 0);
#endif
    return seed;
}

int unique1 = 0, unique2 = 0;

void security_init(void) {
    srand(get_seed());
}

void security_shutdown(void) {
}
