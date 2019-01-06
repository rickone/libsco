#include "cstdio"
#include "asy_scheduler.h"
#include "asy_timer.h"
#include <thread>
#include <vector>
#include <chrono>

using namespace std::chrono_literals;

void foo(int start, int n) {
    for (int i = 0; i < n; ++i) {
        fprintf(stdout, "%04d\n", start + i);
        fflush(stdout);
        
        asy::sleep_for(10ms);
    }
}

int asy_main(int argc, char* argv[]) {
    puts("Hello World!");
    for (int i = 0; i < 10; ++i) {
        asy::scheduler::inst()->start_coroutine(std::bind(foo, i * 2, 2));
    }

    asy::sleep_for(100ms);
    return 0;
}

int main(int argc, char* argv[]) {
    return asy::scheduler::inst()->run(asy_main, argc, argv);
}
