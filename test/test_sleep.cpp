#include <cstdio>
#include <thread>
#include <vector>
#include <chrono>
#include <iostream>
#include "asyn_master.h"

using namespace std::chrono_literals;

void foo(int n) {
    int wid = asyn::worker::current()->id();

    for (int i = 0; i < n; i++) {
        std::cout << '*';
        asyn::sleep_for(10ms);
    }
    std::cout << "worker[" << wid << "] foo(" << n << ")" << std::endl;
}

int main() {
    for (int i = 0; i < 10; i++) {
        std::cout << "ready " << i << std::endl;
        std::this_thread::sleep_for(10ms);
    }

    asyn::guard ag;

    puts("Hello Asyn!");

    std::vector<int> cos;
    for (int i = 1; i <= 10; i++) {
        cos.push_back(asyn::start(std::bind(foo, i)));
    }

    for (int cid : cos) {
        asyn::join(cid);
    }

    return 0;
}
