#include "asyn.h"
#include <cstdio>
#include <thread>
#include <unistd.h>
#include <time.h>

using namespace std::chrono_literals;

void foo0() {
    for (int i = 0; i < 10; i++) {
        printf("asyn::sleep_for i=%d\n", i);
        asyn::sleep_for(10ms);
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
    asyn::start(foo0);
    asyn::start(foo1);
    asyn::start(foo2);
    asyn::start(foo3);
    asyn::start(foo4);

    asyn::sleep_for(11s);
    return 0;
}
