#pragma once

#include <type_traits>
#include "sco_global_queue.h"
#include "sco_mutex.h"
#include "sco_wait_group.h"
#include "sco_channel.h"
#include "sco_except.h"
#include "sco_auto_scheduler.h"

namespace sco {

template<typename F>
inline auto start_impl(const F& f) {
    auto ch = std::make_shared<channel<decltype(f())>>();
    routine::func_t func = [f, ch](){
        auto r = f();
        ch->send(r);
    };
    global_queue::inst()->push_routine(func);
    return ch;
}

template<typename F>
inline void start_impl(const F& f, std::true_type&&) {
    routine::func_t func = f;
    global_queue::inst()->push_routine(func);
}

template<typename F>
inline auto start(const F& f) {
    return start_impl(f, typename std::is_void<decltype(f())>::type());
}

void sleep_until(const std::chrono::steady_clock::time_point& tp);

template <class Rep, class Period>
inline void sleep_for(const std::chrono::duration<Rep, Period>& dtn) {
    auto tp = std::chrono::steady_clock::now() + dtn;
    sleep_until(tp);
}

inline void pause() {
    auto scheduler = scheduler::current();
    if (!scheduler) {
        return;
    }

    scheduler->pause();
}

} // sco
