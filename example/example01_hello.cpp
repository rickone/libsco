#include "asyn.h"
#include <cstdio>

using namespace std::chrono_literals;

void foo(int i) {
    printf("[%d] Hello World!\n", i);
}

int main() {
    for (int i = 0; i < 20; i++) {
        asyn::start(std::bind(foo, i));
        asyn::sleep_for(10ms);
    }
    return 0;
}
