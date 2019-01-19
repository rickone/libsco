#include "asyn.h"
#include <cstdio>

void foo() {
    for (int i = 0; i < 10; i++) {
        asyn::yield("yeah", i);
    }
}

int main() {
    asyn::guard ag;

    int cid = asyn::start(foo);
    while (auto obj = asyn::wait(cid)) {
        auto yeah = obj.load<const char*>();
        auto i = obj.load<int>();
        printf("wait: ('%s', %d)\n", yeah, i);
    }

    fflush(stdout);
	return 0;
}
