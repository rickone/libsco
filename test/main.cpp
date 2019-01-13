#include <cstdio>
#include <thread>
#include <vector>
#include <chrono>
#include <future>
#include <iostream>
#include "asyn_master.h"

using namespace std::chrono_literals;

void foo(int start, int n) {
    for (int i = 0; i < n; ++i) {
        fprintf(stdout, "%04d\n", start + i);
        fflush(stdout);
        
        std::this_thread::sleep_for(10ms);
        //asyn::sleep_for(10ms);
    }
}

int main() {
    asyn::guard ag;

    puts("Hello World!");

    std::vector<int> cos;
    for (int i = 0; i < 10; ++i) {
        cos.push_back(asyn::start(std::bind(foo, i * 2, 2)));
    }

    for (int cid : cos) {
        asyn::join(cid);
    }

    return 0;
}

bool is_prime(int x) {
    for (int i = 2; i < x; ++i) {
        if (x % i == 0) {
            return false;
        }
    }
    return true;
}

int main2() {
    // call function asynchronously:
    std::future<bool> fut = std::async(is_prime, 444444443); 

    // do something while waiting for function to set future:
    std::cout << "checking, please wait";
    /*while (fut.wait_for(100ms) == std::future_status::timeout) {
        std::cout << '.';
    }*/
    fut.wait();

    std::cout << "\n444444443 " << (fut.get() ? "is" : "is not") << " prime.\n";
    return 0;
}
