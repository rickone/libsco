#pragma once

#include <pthread.h>
#include <list>
#include <memory>
#include <unordered_map>
#include "asyn_coroutine.h"
#include "asyn_timer.h"
#include "asyn_poller.h"
#include "asyn_lockfree_queue.h"

namespace asyn {

class worker {
public:
    worker() = default;
    virtual ~worker() = default;

    static worker* current();

    void run(int id);
    void init_context();
    void join();
    void yield(coroutine* co);
    void on_exec();
    void on_command(int type, box::object& obj);
    void on_resume(int cid);

    template<typename... A>
    void command(int type, A... args) {
        box::object obj;
        obj.store(type);
        obj.store(args...);
        _commands.push(std::move(obj));
    }

    coroutine* co_self() { return _self; }
    void set_co_self(coroutine* self) { _self = self; }
    timer* timer_inst() { return &_timer; }
    poller* poller_inst() { return &_poller; }

private:
    int _id = 0;
    pthread_t _thread = nullptr;
    coroutine* _self = nullptr;
    timer _timer;
    poller _poller;
    std::list<std::shared_ptr<coroutine>> _coroutines;
    lockfree_queue<box::object> _commands;
    std::unordered_map<int, std::shared_ptr<coroutine>> _yield_coroutines;
};

enum { // command type
    cmd_resume,
};

} // asyn
