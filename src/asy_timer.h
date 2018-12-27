#pragma once

#include <chrono>
#include "asy_skiplist.h"
#include "asy_coroutine.h"

namespace asy {

class timer {
public:
    using time_point = std::chrono::steady_clock::time_point;

    timer() = default;
    virtual ~timer() = default;

    void sleep(unsigned int ns, coroutine* co);
    void tick();

private:
    skiplist<time_point, coroutine*, std::less<time_point>, 32> _skiplist;
};

void sleep(unsigned int ns);

} // asy
