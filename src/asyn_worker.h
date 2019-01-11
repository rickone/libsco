#pragma once

#include <pthread.h>
#include <list>
#include <memory>
#include <unordered_map>
#include "asyn_coroutine.h"
#include "asyn_timer.h"
#include "asyn_poller.h"
#include "asyn_queue.h"

namespace asyn {

enum {
    exe_resume,
};

class master;

class worker {
public:
    worker() = default;
    virtual ~worker() = default;

    static worker* current();

    void run(int id, master* sche);
    void init_context();
    void join();
    void yield(coroutine* co);
    void on_exec();
    void on_request(int type, box::object& obj);
    void on_resume(int cid);

    template<typename... A>
    void request(int type, A... args) {
        box::object obj;
        obj.store(type);
        obj.store(args...);
        _requests.push(std::move(obj));
    }

    coroutine* co_self() { return _self; }
    void set_co_self(coroutine* self) { _self = self; }
    timer* timer_inst() { return &_timer; }
    poller* poller_inst() { return &_poller; }

private:
    int _id = 0;
    master* _sche = nullptr;
    pthread_t _thread = nullptr;
    coroutine* _self = nullptr;
    timer _timer;
    poller _poller;
    std::list<std::shared_ptr<coroutine>> _coroutines;
    queue<box::object> _requests;
    std::unordered_map<int, std::shared_ptr<coroutine>> _yield_coroutines;
};

} // asyn
