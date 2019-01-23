#include "asyn.h"
#include <cstdio>

using namespace std::chrono_literals;

int foo() {
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        printf("foo i=%d\n", i);
        asyn::sleep_for(10ms);
        sum += i;
    }
    return sum;
}

int main() {
    asyn::guard ag;

    auto fut = asyn::start(foo);
    auto s = fut.wait();

    printf("s=%d\n", s);
    fflush(stdout);
    return 0;
}
