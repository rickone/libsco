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

#define REQUEST_CO_COUNT 8

class worker {
public:
    worker() = default;
    ~worker() = default;

    static worker* current();

    void run(coroutine* self = nullptr);
    void run_in_thread();
    void join();
    void bind_cpu_core(int cpu_core);
    void pause();

    coroutine* co_self() { return _self; }
    void set_co_self(coroutine* self) { _self = self; }
    timer* timer_inst() { return &_timer; }
    poller* poller_inst() { return &_poller; }

private:
    void process_new_coroutines();
    void process_dead_coroutines();
    void process_paused_coroutines();

    pthread_t _thread = (pthread_t)0;
    coroutine* _self = nullptr;
    timer _timer;
    poller _poller;
    std::list<std::shared_ptr<coroutine>> _coroutines;
    std::unordered_map<int, std::shared_ptr<coroutine>> _paused_coroutines;
    int _request_co_count = 0;
};

} // asyn
