#include "asyn_coroutine.h"
#include <cstdlib> // malloc, free
#include <cassert>
#include "asyn_master.h"
#include "asyn_worker.h"

using namespace asyn;

coroutine::coroutine(int id, const func_t& func) : _id(id), _func(func) {
}

coroutine::~coroutine() {
    if (_stack) {
        free(_stack);
    }
}

coroutine* coroutine::self() {
    return worker::current()->co_self();
}

void coroutine::body(coroutine* co) {
    if (co->_func) {
        co->_func();
    }

    co->_status = COROUTINE_DEAD;
}

void coroutine::init(const func_t& func, size_t stack_len) {
    assert(_status == COROUTINE_UNINIT);

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
    worker::current()->set_co_self(this);
}

bool coroutine::resume() {
    if (_status != COROUTINE_SUSPEND) {
        return false;
    }

    _status = COROUTINE_RUNNING;

    auto self = coroutine::self();
    _ctx.uc_link = &self->_ctx;

    set_self();
    swapcontext(&self->_ctx, &_ctx);
    self->set_self();

    return true;
}

void coroutine::yield() {
    _status = COROUTINE_SUSPEND;

    swapcontext(&_ctx, _ctx.uc_link);
}

void coroutine::join() {
    if (_detach) {
        return;
    }

    auto self = coroutine::self();
    if (this == self) {
        return;
    }

    if (_join_id > 0) {
        return;
    }

    _join_id = self->id();
    master::inst()->request(req_join, self->id(), _id);
    worker::current()->yield(self);
}

void coroutine::detach() {
    _detach = true;
}
