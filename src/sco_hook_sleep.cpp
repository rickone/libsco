#include "sco_dlfunc.h"
#include <cstdio>
#include <unistd.h> // usleep
#include <time.h>
#include "sco.h"

using namespace sco;

sys_hook(sleep)
unsigned int sleep(unsigned int seconds) {
    if (!scheduler::current()) {
        return sys_org(sleep)(seconds);
    }

    sco::sleep_for(std::chrono::seconds(seconds));
    return 0;
}

sys_hook(usleep)
int usleep(useconds_t usec) {
    if (!scheduler::current()) {
        return sys_org(usleep)(usec);
    }

    sco::sleep_for(std::chrono::microseconds(usec));
    return 0;
}

sys_hook(nanosleep)
int nanosleep(const struct timespec *req, struct timespec *rem) {
    if (!scheduler::current()) {
        return sys_org(nanosleep)(req, rem);
    }

    int64_t ns = (int64_t)req->tv_sec * 1'000'000'000 + req->tv_nsec;
    sco::sleep_for(std::chrono::nanoseconds(ns));
    return 0;
}
