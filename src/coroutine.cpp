#include "coroutine.h"
#include <cstdlib>

coroutine_t* coroutine_t::main() {
    return nullptr;
}

coroutine_t* coroutine_t::self() {
    return nullptr;
}

coroutine_t* coroutine_t::create(const std::function<void(void)>& func, size_t stack_len) {
    auto co = new coroutine_t();
    co->init(func, stack_len);
    return co;
}

void coroutine_t::run(coroutine_t* co) {
    if (co->_func)
        co->_func();

    co->_status = COROUTINE_DEAD;
}

void coroutine_t::init(const std::function<void(void)>& func, size_t stack_len) {
    _stack = (char*)malloc(stack_len);
    _stack_len = stack_len;

    getcontext(&_ctx);
    _ctx.uc_stack.ss_sp = _stack;
    _ctx.uc_stack.ss_size = stack_len;
    _ctx.uc_link = nullptr;

    if (func) {
        makecontext(&_ctx, (void (*)(void))&coroutine_t::run, 1, this);
        _status = COROUTINE_SUSPEND;
    }
}

void* coroutine_t::resume(void* arg) {
    _arg = arg;
    _status = COROUTINE_RUNNING;

    auto self = coroutine_t::self();
    _ctx.uc_link = &self->_ctx;
    swapcontext(&self->_ctx, &_ctx);

    return _arg;
}

void* coroutine_t::yiled(void* arg) {
    _arg = arg;
    _status = COROUTINE_SUSPEND;

    swapcontext(&_ctx, _ctx.uc_link);

    return _arg;
}
