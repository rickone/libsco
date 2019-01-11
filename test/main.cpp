#include "cstdio"
#include "asyn_master.h"
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

    std::vector<std::shared_ptr<asyn::coroutine>> coroutines;
    for (int i = 0; i < 10; ++i) {
        coroutines.push_back(asyn::master::inst()->start_coroutine(std::bind(foo, i * 2, 2)));
    }

    for (auto& co : coroutines) {
        co->join();
    }

    return 0;
}

int main(int argc, char* argv[]) {
    return asyn::master::inst()->run(asyn_main, argc, argv);
}
