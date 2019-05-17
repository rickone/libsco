#pragma once

#include <type_traits>
#include "sco_global_queue.h"
#include "sco_sleep.h"
#include "sco_channel.h"
#include "sco_mutex.h"
#include "sco_wait_group.h"

namespace sco {

template<typename F>
inline auto start_impl(const F& f, std::false_type&&) {
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

} // sco
