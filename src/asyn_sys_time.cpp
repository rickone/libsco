#include "asyn_dlfunc.h"
#include <cstdio>
#include <unistd.h> // usleep
#include <time.h>
#include "asyn_master.h"

using namespace asyn;

sys_hook(usleep)
int usleep(useconds_t usec) {
    return sys_org(usleep)(usec);
}

sys_hook(sleep)
unsigned int sleep(unsigned int seconds) {
    return sys_org(sleep)(seconds);
}

sys_hook(nanosleep)
int nanosleep(const struct timespec *req, struct timespec *rem) {
    if (!worker::current()) {
        return sys_org(nanosleep)(req, rem);
    }

    puts("asyn --> nanosleep");
    int64_t ns = (int64_t)req->tv_sec * 1'000'000'000 + req->tv_nsec;
    asyn::nsleep(ns);
    return 0;
}
