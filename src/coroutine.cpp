#include "coroutine.h"
#include <cstdlib>
#include <mutex>
#include "pthread.h"

struct coroutine_context {
    coroutine* self = nullptr;
};

static std::once_flag s_init_co_ctx_flag;
static pthread_key_t s_coroutine_context_key;

void purge_co_ctx(void* ctx) {
    puts("purge_co_ctx");
    auto co_ctx = (coroutine_context*)ctx;
    delete co_ctx;
}

void init_coroutine_context_key() {
    pthread_key_create(&s_coroutine_context_key, purge_co_ctx);
}

void coroutine::setup() {
    std::call_once(s_init_co_ctx_flag, init_coroutine_context_key);

    auto ctx = new coroutine_context();
    pthread_setspecific(s_coroutine_context_key, ctx);

    ctx->self = coroutine::create(nullptr);
}

coroutine* coroutine::self() {
    auto ctx = (coroutine_context*)pthread_getspecific(s_coroutine_context_key);
    return ctx ? ctx->self : nullptr;
}

coroutine* coroutine::create(const std::function<void(void)>& func, size_t stack_len) {
    auto co = new coroutine();
    co->init(func, stack_len);
    return co;
}

void coroutine::run(coroutine* co) {
    if (co->_func)
        co->_func();

    co->_status = COROUTINE_DEAD;
}

void coroutine::init(const std::function<void(void)>& func, size_t stack_len) {
    _stack = (char*)malloc(stack_len);
    _stack_len = stack_len;

    getcontext(&_ctx);
    _ctx.uc_stack.ss_sp = _stack;
    _ctx.uc_stack.ss_size = stack_len;
    _ctx.uc_link = nullptr;

    _func = func;
    if (func) {
        makecontext(&_ctx, (void (*)(void))&coroutine::run, 1, this);
        _status = COROUTINE_SUSPEND;
    }
}

void coroutine::set_self() {
    auto ctx = (coroutine_context*)pthread_getspecific(s_coroutine_context_key);
    if (ctx)
        ctx->self = this;
}

void* coroutine::resume(void* arg) {
    if (_status != COROUTINE_SUSPEND)
        return nullptr;

    _arg = arg;
    _status = COROUTINE_RUNNING;

    auto self = coroutine::self();
    _ctx.uc_link = &self->_ctx;

    set_self();
    swapcontext(&self->_ctx, &_ctx);
    self->set_self();

    return _arg;
}

void* coroutine::yield(void* arg) {
    _arg = arg;
    _status = COROUTINE_SUSPEND;

    swapcontext(&_ctx, _ctx.uc_link);

    return _arg;
}
