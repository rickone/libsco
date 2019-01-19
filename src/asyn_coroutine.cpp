#include "asyn_coroutine.h"
#include <cstdlib> // malloc, free
#include <cassert>
#include "asyn_master.h"
#include "asyn_worker.h"

using namespace asyn;

coroutine::coroutine(const func_t& func) : _func(func) {
    static std::atomic<int> s_next_cid;
    _id = ++s_next_cid;
}

coroutine::~coroutine() {
    if (_stack) {
        free(_stack);
    }
}

coroutine* coroutine::self() {
    auto cur_worker = worker::current();
    if (!cur_worker) {
        return nullptr;
    }
    return cur_worker->co_self();
}

void coroutine::body(coroutine* co) {
    if (co->_func) {
        co->_func();
    }

    co->_status = COROUTINE_DEAD;
}

void coroutine::init(const func_t& func, size_t stack_len) {
    if (_status != COROUTINE_UNINIT) {
        return;
    }

    if (func) {
        _func = func;
    }

    getcontext(&_ctx);

    _stack = malloc(stack_len);
    _ctx.uc_stack.ss_sp = _stack;
    _ctx.uc_stack.ss_size = stack_len;
    _ctx.uc_link = nullptr;

    if (_func) {
        makecontext(&_ctx, (void (*)(void))&coroutine::body, 1, this);
        _status = COROUTINE_SUSPEND;
    } else {
        _status = COROUTINE_RUNNING;
    }
}

void coroutine::set_self() {
    auto cur_worker = worker::current();
    if (cur_worker) {
        cur_worker->set_co_self(this);
    }
}

void coroutine::swap(coroutine* co) {
    _status = COROUTINE_SUSPEND;
    co->_status = COROUTINE_RUNNING;
    swapcontext(&_ctx, &co->_ctx);
}

bool coroutine::resume() {
    if (_status != COROUTINE_SUSPEND) {
        return false;
    }

    auto self = coroutine::self();
    if (!self) {
        return false; // panic
    }

    _ctx.uc_link = &self->_ctx;
    set_self();
    self->swap(this);
    self->set_self();
    return true;
}

void coroutine::yield() {
    if (!_ctx.uc_link) {
        return; // panic
    }

    _status = COROUTINE_SUSPEND;
    swapcontext(&_ctx, _ctx.uc_link);
}

void coroutine::yield_return() {
    if (!_ctx.uc_link) {
        return; // panic
    }

    _status = COROUTINE_DEAD;
    swapcontext(&_ctx, _ctx.uc_link);
}
