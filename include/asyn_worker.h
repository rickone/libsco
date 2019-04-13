#pragma once

#include <pthread.h>
#include <list>
#include <memory>
#include <unordered_map>
#include "asyn_event.h"
#include "asyn_coroutine.h"

namespace asyn {

#define REQUEST_CO_COUNT 8

class worker : public event_trigger {
public:
    worker() = default;
    virtual ~worker();

    static worker* current();

    void run(coroutine* self = nullptr);
    void run_in_thread();
    void join();
    void bind_cpu_core(int cpu_core);
    void pause();

    virtual void on_event(evutil_socket_t fd, int flag) override;

    coroutine* co_self() { return _self; }
    void set_co_self(coroutine* self) { _self = self; }
    struct event_base* event_inst() { return _event_base; }

private:
    void process_new_coroutines();
    void process_dead_coroutines();
    void process_paused_coroutines();

    pthread_t _thread = (pthread_t)0;
    coroutine* _self = nullptr;
    std::list<std::shared_ptr<coroutine>> _coroutines;
    std::unordered_map<int, std::shared_ptr<coroutine>> _paused_coroutines;
    int _request_co_count = 0;
    struct event_base* _event_base = nullptr;
};

} // asyn
