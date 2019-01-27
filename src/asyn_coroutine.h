#pragma once

#include <functional>
#include <memory>
#include <ucontext.h>
#include "box.h"
#include "asyn_iterator.h"

namespace asyn {

const int COROUTINE_DEFAULT_STACK_LEN = 1024 * 64;

enum {
    COROUTINE_UNINIT,
    COROUTINE_RUNNING,
    COROUTINE_SUSPEND,
    COROUTINE_DEAD,
};

class coroutine : public std::enable_shared_from_this<coroutine> {
public:
    typedef std::function<void(void)> func_t;
    
    explicit coroutine(const func_t& func);
    ~coroutine();
    coroutine(const coroutine&) = delete;
    coroutine(coroutine&&) = delete;
    coroutine& operator=(const coroutine&) = delete;
    coroutine& operator=(coroutine&&) = delete;

    static coroutine* self();

    void init(size_t stack_len = COROUTINE_DEFAULT_STACK_LEN);
    void set_self();
    void swap(coroutine* co);
    bool resume();
    void yield();
    void yield_return();
    iterator begin();
    iterator end();

    box::object get_value() { return std::move(_val); }

    template<typename... A>
    void set_value(A... args) {
        _val.store_args(args...);
    }

    void move_value(box::object&& val) {
        _val = std::move(val);
    }

    int id() const { return _id; }
    int status() const { return _status; }

private:
    static void body(coroutine* co);

    int _id = 0;
    func_t _func = nullptr;
    void* _stack = nullptr;
    ucontext_t _ctx;
    int _status = COROUTINE_UNINIT;
    box::object _val;
};

template<typename... A>
inline box::object resume(coroutine& co, A... args) {
    if (co.status() != COROUTINE_SUSPEND)
        return nullptr;

    co.set_value(args...);
    co.resume();
    return co.get_value();
}

template<typename... A>
inline box::object yield(A... args) {
    auto self = coroutine::self();
    self->set_value(args...);
    self->yield();
    return self->get_value();
}

template<typename... A>
inline void yield_return(A... args) {
    auto self = coroutine::self();
    self->set_value(args...);
    self->yield_return();
}

} // asyn
