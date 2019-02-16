#include "asyn.h"
#include <cstdio>

void foo() {
    for (int i = 0; i < 10; i++) {
        asyn::yield("yeah", i);

        if (i == 5) {
            asyn::yield_break("asyn", 100);
        }
    }
}

int main() {
    asyn::coroutine co(foo);
    while (auto obj = asyn::resume(co)) {
        auto yeah = obj.load<const char*>();
        auto i = obj.load<int>();
        printf("resume: ('%s', %d)\n", yeah, i);
    }
    return 0;
}
