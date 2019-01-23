#pragma once

#include "asyn_master.h"
#include "asyn_mutex.h"
#include "asyn_wait_group.h"
#include "asyn_future.h"

namespace asyn {

class guard {
public:
    guard() {
        master::inst()->enter();
    }
    ~guard() {
        master::inst()->quit();
    }
};

template<typename R>
inline future<R> start(const std::function<R ()>& f) {
    future<R> fut;
    coroutine::func_t func = [&, f](){
        R r = f();
        fut.done(r);
    };
    master::inst()->start_coroutine(func);
    return fut;
}

template<typename R>
inline future<R> start(R (*f)()) {
    future<R> fut;
    coroutine::func_t func = [&, f](){
        R r = f();
        fut.done(r);
    };
    master::inst()->start_coroutine(func);
    return fut;
}

inline future<void> start(const std::function<void ()>& f) {
    future<void> fut;
    coroutine::func_t func = [&, f](){
        f();
        fut.done();
    };
    master::inst()->start_coroutine(func);
    return fut;
}

inline future<void> start(void (*f)()) {
    future<void> fut;
    coroutine::func_t func = [&, f](){
        f();
        fut.done();
    };
    master::inst()->start_coroutine(func);
    return fut;
}

template <class Rep, class Period>
inline void sleep_for(const std::chrono::duration<Rep, Period>& dtn) {
    auto nsdtn = std::chrono::duration_cast<std::chrono::nanoseconds>(dtn);
    nsleep(nsdtn.count());
}

void pause() {
    auto cur_worker = worker::current();
    if (!cur_worker) {
        return;
    }

    cur_worker->pause();
}

} // asyn
