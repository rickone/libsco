#pragma once

#include <functional>
#include <ucontext.h>

#define COROUTINE_DEFAULT_STACK_LEN (1024 * 256)

enum {
    COROUTINE_RUNNING,
    COROUTINE_SUSPEND,
    COROUTINE_DEAD,
};

class coroutine_t {
public:
    coroutine_t() = default;
    virtual ~coroutine_t() = default;

    static coroutine_t* main();
    static coroutine_t* self();
    static coroutine_t* create(const std::function<void(void)>& func, size_t stack_len = COROUTINE_DEFAULT_STACK_LEN);
    static void run(coroutine_t* co);

    void init(const std::function<void(void)>& func, size_t stack_len);
    void* resume(void* arg);
    void* yiled(void* arg);

    int status() const { return _status; }

private:
    char* _stack = nullptr;
    size_t _stack_len = 0;
    ucontext_t _ctx;
    void* _arg = nullptr;
    std::function<void(void)> _func;
    int _status = COROUTINE_RUNNING;
};
