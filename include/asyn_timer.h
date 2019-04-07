#pragma once

#include <chrono>
#include "asyn_skiplist.h"

namespace asyn {

#define MIN_TRIGGER_NANO_SECONDS 10'000'000ll

class timer {
public:
    struct trigger {
        virtual void on_timer() = 0;
    };

    using time_point = std::chrono::steady_clock::time_point;

    timer() = default;
    ~timer() = default;

    void add_trigger(const time_point& tp, trigger* trigger);
    void remove_trigger(const time_point& tp, trigger* trigger);
    int64_t tick();

private:
    skiplist<time_point, trigger*, std::less<time_point>, 32> _skiplist;
};

} // asyn
