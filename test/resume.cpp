#include "sco.h"
#include <cstdio>

void foo() {
    for (int i = 0; i < 10; i++) {
        sco::yield("yeah", i);

        if (i == 5) {
            sco::yield_break("sco", 100);
        }
    }
}

int main() {
    sco::coroutine co(foo);
    while (auto obj = sco::resume(co)) {
        auto yeah = obj.load<const char*>();
        auto i = obj.load<int>();
        printf("resume: ('%s', %d)\n", yeah, i);
    }
    return 0;
}
