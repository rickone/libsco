#include "cstdio"
#include "asy_scheduler.h"
#include "asy_timer.h"

void foo(int start, int n) {
    for (int i = 0; i < n; ++i) {
        fprintf(stdout, "%04d\n", start + i);
        fflush(stdout);
        
        asy::sleep(100'000'000);
    }
    fprintf(stderr, "foo[%d] exit\n", start);
}

int main() {
    auto sch = asy::scheduler::inst();
    for (int i = 0; i < 100; ++i) {
        sch->push_func(std::bind(foo, i * 100, 100));
    }
    sch->run();
    return 0;
}
