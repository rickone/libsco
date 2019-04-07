#include "asyn_dlfunc.h"
#include <cstdio>
#include <unistd.h> // usleep
#include <time.h>
#include "asyn.h"

using namespace asyn;

sys_hook(sleep)
unsigned int sleep(unsigned int seconds) {
    if (!worker::current()) {
        return sys_org(sleep)(seconds);
    }

    asyn::sleep_for(std::chrono::seconds(seconds));
    return 0;
}

sys_hook(usleep)
int usleep(useconds_t usec) {
    if (!worker::current()) {
        return sys_org(usleep)(usec);
    }

    asyn::sleep_for(std::chrono::microseconds(usec));
    return 0;
}

sys_hook(nanosleep)
int nanosleep(const struct timespec *req, struct timespec *rem) {
    if (!worker::current()) {
        return sys_org(nanosleep)(req, rem);
    }

    int64_t ns = (int64_t)req->tv_sec * 1'000'000'000 + req->tv_nsec;
    asyn::sleep_for(std::chrono::nanoseconds(ns));
    return 0;
}
