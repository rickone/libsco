#pragma once

#include <functional>
#include <ucontext.h>

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
    void* resume(void* arg = nullptr);
    void* yield(void* arg = nullptr);

    int status() const { return _status; }

private:
    static void run(coroutine* co);

    char* _stack = nullptr;
    size_t _stack_len = 0;
    ucontext_t _ctx;
    void* _arg = nullptr;
    std::function<void(void)> _func;
    int _status = COROUTINE_RUNNING;
};

#define co_create(func) coroutine::create(func)
#define co_yield() coroutine::self()->yield()
