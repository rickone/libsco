#pragma once

#include <functional>
#include <memory>
#include <ucontext.h>
#include "sco_event.h"

namespace sco {

const int COROUTINE_DEFAULT_STACK_LEN = 1024 * 64;

enum {
    COROUTINE_UNINIT,
    COROUTINE_RUNNING,
    COROUTINE_SUSPEND,
    COROUTINE_DEAD,
};

class routine : public std::enable_shared_from_this<routine>, public event_trigger {
public:
    typedef std::function<void(void)> func_t;
    
    explicit routine(const func_t& func);
    virtual ~routine();
    routine(const routine&) = delete;
    routine(routine&&) = delete;
    routine& operator=(const routine&) = delete;
    routine& operator=(routine&&) = delete;

    static routine* self();

    void init(size_t stack_len = COROUTINE_DEFAULT_STACK_LEN);
    void set_self();
    void swap(routine* co);
    bool resume();
    void yield();
    void yield_break();
    int wait_event(evutil_socket_t fd, int flag, int64_t timeout_usec);

    virtual void on_event(evutil_socket_t fd, int flag) override;

    int id() const { return _id; }
    int status() const { return _status; }

private:
    static void body(routine* co);

    int _id = 0;
    func_t _func = nullptr;
    void* _stack = nullptr;
    ucontext_t _ctx;
    int _status = COROUTINE_UNINIT;
    int _event_flag = 0;
};

} // sco
