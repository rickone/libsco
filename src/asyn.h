#pragma once

#include "asyn_master.h"
#include "asyn_mutex.h"
#include "asyn_wait_group.h"
#include "asyn_channel.h"
#include "asyn_panic.h"

namespace asyn {

class guard {
public:
    guard();
    ~guard();
};

extern guard g_guard_inst;

inline void start(const std::function<void ()>& f) {
    master::inst()->start_coroutine(f);
}

inline void start(void (*f)()) {
    master::inst()->start_coroutine(f);
}

template<typename R>
inline std::shared_ptr<channel> start(const std::function<R ()>& f) {
    auto ch = std::make_shared<channel>();
    coroutine::func_t func = [f, ch](){
        R r = f();
        ch->send(r);
    };
    master::inst()->start_coroutine(func);
    return ch;
}

template<typename R>
inline std::shared_ptr<channel> start(R (*f)()) {
    auto ch = std::make_shared<channel>();
    coroutine::func_t func = [f, ch](){
        R r = f();
        ch->send(r);
    };
    master::inst()->start_coroutine(func);
    return ch;
}

template <class Rep, class Period>
inline void sleep_for(const std::chrono::duration<Rep, Period>& dtn) {
    auto nsdtn = std::chrono::duration_cast<std::chrono::nanoseconds>(dtn);
    nsleep(nsdtn.count());
}

inline void pause() {
    auto cur_worker = worker::current();
    if (!cur_worker) {
        return;
    }

    cur_worker->pause();
}

inline void quit(int code) {
    master::inst()->quit(code);
    coroutine::self()->yield_return();
}

} // asyn
