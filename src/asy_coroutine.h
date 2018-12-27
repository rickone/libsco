#pragma once

#include <functional>
#include <memory>
#include <ucontext.h>
#include "xbin.h"

namespace asy {

const int COROUTINE_DEFAULT_STACK_LEN = 1024 * 256;

enum {
    COROUTINE_RUNNING,
    COROUTINE_SUSPEND,
    COROUTINE_DEAD,
};

class coroutine {
public:
    explicit coroutine(size_t stack_len = COROUTINE_DEFAULT_STACK_LEN);
    virtual ~coroutine();

    static coroutine* self();

    void bind(const std::function<void(void)>& func);
    void set_self();
    bool resume();
    void yield();

    xbin::object get_value() { return std::move(_val); }

    template<typename... A>
    void set_value(A... args) {
        _val.store_args(args...);
    }

    void move_value(xbin::object&& val) {
        _val = std::move(val);
    }

    int status() const { return _status; }

private:
    static void body(coroutine* co);

    void* _stack = nullptr;
    ucontext_t _ctx;
    xbin::object _val;
    std::function<void(void)> _func;
    int _status;
};

template<typename R, typename... A>
inline std::shared_ptr<coroutine> create_coroutine(const std::function<R (A...)>& func, size_t stack_len = COROUTINE_DEFAULT_STACK_LEN) {
    auto co = std::make_shared<coroutine>(stack_len);
    std::function<void(void)> real_func = [co, func](){
        auto val = co->get_value();
        auto r = val.call(func);
        co->set_value(r);
    };
    co->bind(real_func);
    return co;
}

template<typename R, typename... A>
inline std::shared_ptr<coroutine> create_coroutine(R (*func)(A...), size_t stack_len = COROUTINE_DEFAULT_STACK_LEN) {
    auto co = std::make_shared<coroutine>(stack_len);
    std::function<void(void)> real_func = [co, func](){
        auto val = co->get_value();
        auto r = val.call(func);
        co->set_value(r);
    };
    co->bind(real_func);
    return co;
}

template<typename... A>
inline std::shared_ptr<coroutine> create_coroutine(const std::function<void (A...)>& func, size_t stack_len = COROUTINE_DEFAULT_STACK_LEN) {
    auto co = std::make_shared<coroutine>(stack_len);
    std::function<void(void)> real_func = [co, func](){
        auto val = co->get_value();
        val.invoke(func);
    };
    co->bind(real_func);
    return co;
}

template<typename... A>
inline std::shared_ptr<coroutine> create_coroutine(void (*func)(A...), size_t stack_len = COROUTINE_DEFAULT_STACK_LEN) {
    auto co = std::make_shared<coroutine>(stack_len);
    std::function<void(void)> real_func = [co, func](){
        auto val = co->get_value();
        val.invoke(func);
    };
    co->bind(real_func);
    return co;
}

template<typename... A>
inline xbin::object yield(A... args) {
    auto co = coroutine::self();
    co->set_value(args...);
    co->yield();
    return co->get_value();
}

template<typename... A>
inline xbin::object resume(const std::shared_ptr<coroutine>& co, A... args) {
    if (co->status() != COROUTINE_SUSPEND)
        return xbin::object();

    co->set_value(args...);
    co->resume();
    return co->get_value();
}

} // asy
