#include "asyn.h"
#include <cstdio>
#include <cassert>

bool is_prime(int n) {
    if (n <= 2) {
        return true;
    }

    if (n % 2 == 0) {
        return false;
    }

    int m = (int)sqrt(n);
    for (int i = 3; i <= m; i += 2) {
        if (n % i == 0) {
            return false;
        }
    }

    return true;
}

void hello() {
    puts("Hello World!");
}

void foo(int start, int n) {
    for (int i = 0; i < n; i++) {
        int x = start + i;
        if (is_prime(x)) {
            asyn::yield(x);
        }
    }
}

void test() {
    for (int i = 0; i < 10; ++i) {
        if (i == 5) {
            asyn::yield_break(100);
        }

        asyn::yield(i * i);
    }
}

int main() {
    for (auto& obj : asyn::coroutine(hello)) {
        obj.clear();
        assert(false);
    }

    for (auto& obj : asyn::coroutine(std::bind(foo, 2, 1000))) {
        printf("prime=%d\n", obj.load<int>());
    }

    for (auto& obj : asyn::coroutine(test)) {
        printf("n=%d\n", obj.load<int>());
    }
    return 0;
}
