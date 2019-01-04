#include "cstdio"
#include "asy_scheduler.h"
#include "asy_timer.h"
#include <thread>
#include <vector>
#include <chrono>

using namespace std::chrono_literals;

void foo(int start, int n) {
    /*
    for (int i = 0; i < n; ++i) {
        fprintf(stdout, "%04d\n", start + i);
        fflush(stdout);
        
        std::this_thread::sleep_for(10ms);
    }
    */
    for (int i = 0; i < n; ++i) {
        fprintf(stdout, "%04d\n", start + i);
        fflush(stdout);
        
        //asy::sleep(10'000'000);
    }
}

int asy_main(int argc, char* argv[]) {
    /*std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back(foo, i * 2, 2);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    */
    puts("Hello World!");
    /*for (int i = 0; i < 10; ++i) {
        asy::scheduler::inst()->create_coroutine(std::bind(foo, i * 2, 2));
    }*/

    //asy::sleep(10'000'000 * 4);
    return 0;
}

int main(int argc, char* argv[]) {
    return asy::scheduler::inst()->run(asy_main, argc, argv);
}
