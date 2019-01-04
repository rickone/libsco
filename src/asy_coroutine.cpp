#include "asy_coroutine.h"
#include <cstdlib>
#include <mutex>
#include <cassert>
#include "asy_scheduler.h"
#include "asy_context.h"

using namespace asy;

coroutine::coroutine(int id, const func_t& func) : _id(id), _func(func) {
}

coroutine::~coroutine() {
    if (_stack) {
        free(_stack);
    }
}

coroutine* coroutine::self() {
    auto ctx = get_context();
    return ctx ? ctx->co : nullptr;
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
    auto ctx = get_context();
    if (ctx == nullptr) {
        throw std::runtime_error("thread.ctx is null");
    }
    
    ctx->co = this;
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
