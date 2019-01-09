#include "cstdio"
#include "asyn_scheduler.h"
#include "asyn_timer.h"
#include <thread>
#include <vector>
#include <chrono>

using namespace std::chrono_literals;

void foo(int start, int n) {
    for (int i = 0; i < n; ++i) {
        fprintf(stdout, "%04d\n", start + i);
        fflush(stdout);
        
        asyn::sleep_for(10ms);
    }
}

int asyn_main(int argc, char* argv[]) {
    puts("Hello World!");
    for (int i = 0; i < 10; ++i) {
        asyn::scheduler::inst()->start_coroutine(std::bind(foo, i * 2, 2));
    }

    asyn::sleep_for(100ms);
    return 0;
}

int main(int argc, char* argv[]) {
    return asyn::scheduler::inst()->run(asyn_main, argc, argv);
}
