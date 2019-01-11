#include "asyn_override.h"
#include <cstdio>
#include <unistd.h> // usleep
#include <time.h>

ASYN_OVERRIDE(usleep)
int usleep(useconds_t usec) {
    puts("usleep hooked");
    return ASYN_ORIGIN(usleep)(usec);
}

ASYN_OVERRIDE(sleep)
unsigned int sleep(unsigned int seconds) {
    puts("sleep hooked");
    return ASYN_ORIGIN(sleep)(seconds);
}

ASYN_OVERRIDE(nanosleep)
int nanosleep(const struct timespec *req, struct timespec *rem) {
    puts("nanosleep hooked");
    return ASYN_ORIGIN(nanosleep)(req, rem);
}
