#include "asyn.h"
#include <cstdio>

using namespace std::chrono_literals;

void foo() {
    for (int i = 0; i < 10; i++) {
        printf("foo i=%d\n", i);
        asyn::sleep_for(10ms);
    }
}

int main() {
    asyn::guard ag;

    int cid = asyn::start(foo);
    asyn::wait(cid);

    fflush(stdout);
	return 0;
}
