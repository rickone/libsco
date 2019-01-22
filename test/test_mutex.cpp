#include "asyn.h"
#include <cstdio>

static asyn::mutex s_mutex;
static std::vector<int> s_result;

bool is_prime(int n) {
    if (n % 2 == 0) {
        return false;
    }

    int m = (int)sqrt(n);
    for (int i = 3; i <= m; i += 2) {
        if (n % i == 0) {
            return false;
        }
    }

    printf("%d is prime\n", n);
    return true;
}

void foo(int n, asyn::wait_group* wg) {
    if (!is_prime(n)) {
        wg->done();
        return;
    }

    s_mutex.lock();
    s_result.push_back(n);
    s_mutex.unlock();
    wg->done();
}

int main() {
    asyn::guard ag;

    asyn::wait_group wg;
    wg.add(998);

    std::vector<int> cos;
    for (int i = 2; i < 1000; i++) {
        cos.push_back(asyn::start(std::bind(foo, i, &wg)));
    }

    wg.wait();

    puts("prime number:");
    for (int n : s_result) {
        printf("%d\n", n);
    }

    return 0;
}
