#pragma once

#include <type_traits>
#include "asyn_master.h"
#include "asyn_mutex.h"
#include "asyn_wait_group.h"
#include "asyn_channel.h"
#include "asyn_except.h"

namespace asyn {

class guard {
public:
    guard();
    ~guard();
};

extern guard g_guard_inst;

template<typename F, typename C>
inline std::shared_ptr<channel> start_impl(const F& f, C&&) {
    auto ch = std::make_shared<channel>();
    coroutine::func_t func = [f, ch](){
        auto r = f();
        ch->send(r);
    };
    master::inst()->start_coroutine(func);
    return ch;
}

template<typename F>
inline std::shared_ptr<channel> start_impl(const F& f, std::true_type&&) {
    coroutine::func_t func = f;
    master::inst()->start_coroutine(func);
    return nullptr;
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
    auto worker = worker::current();
    if (!worker) {
        return;
    }

    worker->pause();
}

inline void quit(int code) {
    master::inst()->quit(code);
    coroutine::self()->yield_break();
}

} // asyn
