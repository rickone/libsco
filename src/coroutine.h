#pragma once

#include <functional>
#include <ucontext.h>
#include "xbin.h"

#define COROUTINE_DEFAULT_STACK_LEN (1024 * 256)

enum {
    COROUTINE_RUNNING,
    COROUTINE_SUSPEND,
    COROUTINE_DEAD,
};

class coroutine {
public:
    coroutine() = default;
    virtual ~coroutine() = default;

    static void setup();
    static coroutine* self();
    static coroutine* create(const std::function<void(void)>& func, size_t stack_len = COROUTINE_DEFAULT_STACK_LEN);

    void init(const std::function<void(void)>& func, size_t stack_len);
    void set_self();
    xbin::object resume();
    xbin::object yield();

    template<typename... A>
    void set_args(A... args) {
        _arg.store_args(args...);
    }

    int status() const { return _status; }

private:
    static void run(coroutine* co);

    char* _stack = nullptr;
    size_t _stack_len = 0;
    ucontext_t _ctx;
    xbin::object _arg;
    std::function<void(void)> _func;
    int _status = COROUTINE_RUNNING;
};

#define co_create(func) coroutine::create(func)

template<typename... A>
inline xbin::object co_yield(A... args) {
    auto co = coroutine::self();
    co->set_args(args...);
    return co->yield();
}

template<typename... A>
inline xbin::object co_resume(coroutine* co, A... args) {
    if (co->status() != COROUTINE_SUSPEND)
        return xbin::object();

    co->set_args(args...);
    return co->resume();
}
