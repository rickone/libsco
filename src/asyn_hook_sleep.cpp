#include "asyn_dlfunc.h"
#include <cstdio>
#include <unistd.h> // usleep
#include <time.h>
#include "asyn_master.h"

using namespace asyn;

sys_hook(usleep)
int usleep(useconds_t usec) {
    if (!worker::current()) {
        return sys_org(usleep)(usec);
    }

    int64_t ns = usec * 1'000;
    asyn::nsleep(ns);
    return 0;
}

sys_hook(sleep)
unsigned int sleep(unsigned int seconds) {
    if (!worker::current()) {
        return sys_org(sleep)(seconds);
    }

    int64_t ns = seconds * 1'000'000'000;
    asyn::nsleep(ns);
    return 0;
}

sys_hook(nanosleep)
int nanosleep(const struct timespec *req, struct timespec *rem) {
    if (!worker::current()) {
        return sys_org(nanosleep)(req, rem);
    }

    int64_t ns = (int64_t)req->tv_sec * 1'000'000'000 + req->tv_nsec;
    asyn::nsleep(ns);
    return 0;
}
