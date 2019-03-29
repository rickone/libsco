#include "asyn.h"
#include <cstdio>

using namespace std::chrono_literals;

int foo(int start, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        int x = start + i;
        printf("foo i=%d\n", i);
        asyn::sleep_for(10ms);
        sum += x;
    }
    return sum;
}

int main() {
    auto ch = asyn::start(std::bind(foo, 1, 100));
    auto s = ch->wait<int>();

    printf("s=%d\n", s);
    return 0;
}
