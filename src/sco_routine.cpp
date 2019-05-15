#include "sco_routine.h"
#include <cstdlib> // malloc, free
#include <cassert>
//#include "sco_master.h"
#include "sco_scheduler.h"
#include "sco_except.h"

using namespace sco;

routine::routine(const func_t& func) : _func(func) {
    static std::atomic<int> s_next_cid;
    _id = ++s_next_cid;
}

routine::~routine() {
    if (_stack) {
        free(_stack);
    }
}

routine* routine::self() {
    auto scheduler = scheduler::current();
    if (!scheduler) {
        return nullptr;
    }
    return scheduler->co_self();
}

void routine::body(routine* co) {
    if (co->_func) {
        co->_func();
    }

    co->_status = COROUTINE_DEAD;
}

void routine::init(size_t stack_len) {
    if (_status != COROUTINE_UNINIT) {
        return;
    }

    if (_func) {
        auto self = routine::self();

        getcontext(&_ctx);

        _stack = malloc(stack_len);
        _ctx.uc_stack.ss_sp = _stack;
        _ctx.uc_stack.ss_size = stack_len;
        _ctx.uc_link = self ? &self->_ctx : nullptr;

        makecontext(&_ctx, (void (*)(void))&routine::body, 1, this);
        _status = COROUTINE_SUSPEND;
    } else {
        _status = COROUTINE_RUNNING;
    }
}

void routine::set_self() {
    auto scheduler = scheduler::current();
    if (scheduler) {
        scheduler->set_co_self(this);
    }
}

void routine::swap(routine* co) {
    _status = COROUTINE_SUSPEND;
    co->_status = COROUTINE_RUNNING;
    swapcontext(&_ctx, &co->_ctx);
}

bool routine::resume() {
    if (_status == COROUTINE_UNINIT) {
        init();
    }
    
    if (_status != COROUTINE_SUSPEND) {
        return false;
    }

    auto self = routine::self();
    runtime_assert(self, "");

    _ctx.uc_link = &self->_ctx;
    set_self();
    self->swap(this);
    self->set_self();
    return true;
}

void routine::yield() {
    runtime_assert(_ctx.uc_link, "");

    _status = COROUTINE_SUSPEND;
    swapcontext(&_ctx, _ctx.uc_link);
}

void routine::yield_break() {
    runtime_assert(_ctx.uc_link, "");

    _status = COROUTINE_DEAD;
    swapcontext(&_ctx, _ctx.uc_link);
}

int routine::wait_event(evutil_socket_t fd, int flag, int64_t timeout_usec) {
    runtime_assert(_status == COROUTINE_RUNNING, "_status=%d", _status);

    add_event(fd, flag, timeout_usec, this);
    yield();
    return _event_flag;
}

void routine::on_event(evutil_socket_t fd, int flag) {
    _event_flag = flag;
    resume();
}
