#include "asy_override.h"
#include <cstdio>
#include <unistd.h> // usleep
#include <time.h>

ASY_OVERRIDE(usleep)
int usleep(useconds_t usec) {
    puts("usleep hooked");
    return ASY_ORIGIN(usleep)(usec);
}

ASY_OVERRIDE(sleep)
unsigned int sleep(unsigned int seconds) {
    puts("sleep hooked");
    return ASY_ORIGIN(sleep)(seconds);
}

ASY_OVERRIDE(nanosleep)
int nanosleep(const struct timespec *req, struct timespec *rem) {
    puts("nanosleep hooked");
    return ASY_ORIGIN(nanosleep)(req, rem);
}
