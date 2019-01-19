#pragma once

#include <chrono>
#include "asyn_skiplist.h"
#include "asyn_coroutine.h"

namespace asyn {

class timer {
public:
	using time_point = std::chrono::steady_clock::time_point;

    timer() = default;
    ~timer() = default;

    void sleep(int64_t ns, coroutine* co);
    void tick();

private:
    skiplist<time_point, coroutine*, std::less<time_point>, 32> _skiplist;
};

void nsleep(int64_t ns);

} // asyn
