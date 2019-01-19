#pragma once

#include "asyn_master.h"
#include "asyn_mutex.h"

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

inline int start(const std::function<void ()>& func) {
    auto co = master::inst()->start_coroutine(func);
    return co->id();
}

template <class Rep, class Period>
inline void sleep_for(const std::chrono::duration<Rep, Period>& dtn) {
    auto nsdtn = std::chrono::duration_cast<std::chrono::nanoseconds>(dtn);
    nsleep(nsdtn.count());
}

template<typename... A>
inline box::object resume(int cid, A... args) {
    auto cur_worker = worker::current();
    if (!cur_worker) {
        return nullptr;
    }

    box::object obj;
    obj.store_args(args...);
    return cur_worker->resume(cid, obj);
}

template<typename... A>
inline box::object yield(A... args) {
    auto cur_worker = worker::current();
    if (!cur_worker) {
        return nullptr;
    }

    box::object obj;
    obj.store_args(args...);
    return cur_worker->yield(obj);
}

template<typename... A>
inline void yield_return(A... args) {
    auto cur_worker = worker::current();
    if (!cur_worker) {
        return;
    }

    box::object obj;
    obj.store_args(args...);
    cur_worker->yield_return(obj);
}

inline box::object wait(int cid) {
    auto cur_worker = worker::current();
    if (!cur_worker) {
        return nullptr;
    }

    return cur_worker->resume(cid, nullptr);
}

} // asyn
