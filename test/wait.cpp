#include "sco.h"
#include <cstdio>
#include <chrono>

using namespace std::chrono_literals;

int foo(int start, int n) {
    int sum = 0;
    auto begin = std::chrono::steady_clock::now();
    for (int i = 0; i < n; i++) {
        int x = start + i;
        //printf("foo i=%d\n", i);
        //sco::sleep_for(10ms);
        sco::sleep_until(begin + std::chrono::milliseconds(10 * (i + 1)));
        sum += x;
    }
    return sum;
}

void test_wait() {
    auto ch = sco::start(std::bind(foo, 1, 100));
    auto s = ch->wait<int>();

    printf("s=%d\n", s);
}

template <class Rep, class Period>
void test_wait_for(const std::chrono::duration<Rep, Period>& dtn) {
    auto ch = sco::start(std::bind(foo, 1, 100));
    int s = 0;
    
    bool succ = ch->wait_for(dtn, s);
    if (!succ) {
        printf("wait_for timeout\n");
        return;
    }

    printf("s=%d\n", s);
}

int main() {
    test_wait();
    test_wait_for(500ms);
    test_wait_for(1100ms);
    return 0;
}
