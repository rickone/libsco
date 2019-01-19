#include "asyn.h"
#include <cstdio>
#include <cmath>

void sqrt_svr() {
    while (true) {
        auto obj = asyn::yield();
        double x = obj.load<double>();
        double s = sqrt(x);
        asyn::yield(s);
    }
}

void foo(int svr_cid) {
    for (int i = 0; i < 10; i++) {
        asyn::resume(svr_cid, (double)i);
        auto obj = asyn::resume(svr_cid);
        asyn::yield(i, obj.load<double>());
    }
}

int main() {
    asyn::guard ag;

    int sqrt_cid = asyn::start(sqrt_svr);

    int cid = asyn::start(std::bind(foo, sqrt_cid));
    while (auto obj = asyn::wait(cid)) {
        auto i = obj.load<int>();
        auto s = obj.load<double>();
        printf("sqrt(%d) = %lf\n", i, s);
    }

    fflush(stdout);
	return 0;
}
