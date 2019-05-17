#include "sco.h"
#include <cstdio>
#include <thread>
#include <unistd.h>
#include <time.h>

sco::scheduler::auto_attach g_saa;

using namespace std::chrono_literals;

void foo0() {
    for (int i = 0; i < 10; i++) {
        printf("sco::sleep_for i=%d\n", i);
        sco::sleep_for(10ms);
    }
}

void foo1() {
    for (int i = 0; i < 10; i++) {
        printf("std::this_thread::sleep_for i=%d\n", i);
        std::this_thread::sleep_for(10ms);
    }
}

void foo2() {
    for (int i = 0; i < 10; i++) {
        printf("sleep i=%d\n", i);
        sleep(1);
    }
}

void foo3() {
    for (int i = 0; i < 10; i++) {
        printf("usleep i=%d\n", i);
        usleep(10'000);
    }
}

void foo4() {
    struct timespec req = {.tv_sec = 0, .tv_nsec = 10'000'000};
    for (int i = 0; i < 10; i++) {
        printf("nanosleep i=%d\n", i);
        nanosleep(&req, nullptr);
    }
}

int main() {
    //sco::start(foo0);
    //sco::start(foo1);
    sco::start(foo2);
    sco::start(foo3);
    //sco::start(foo4);

    sco::sleep_for(11s);
    return 0;
}
