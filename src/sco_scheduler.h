#pragma once

#include <pthread.h>
#include <list>
#include <memory>
#include <unordered_map>
#include "sco_event.h"
#include "sco_routine.h"

namespace sco {

#define REQUEST_CO_COUNT 8

class scheduler : public event_trigger {
public:
    scheduler() = default;
    virtual ~scheduler();

    static scheduler* current();

    void run(routine* self = nullptr);
    void run_in_thread();
    void join();
    void bind_cpu_core(int cpu_core);
    void pause();

    virtual void on_event(evutil_socket_t fd, int flag) override;

    routine* co_self() { return _self; }
    void set_co_self(routine* self) { _self = self; }
    struct event_base* event_inst() { return _event_base; }

private:
    void process_new_routines();
    void process_dead_routines();
    void process_paused_routines();

    pthread_t _thread = (pthread_t)0;
    routine* _self = nullptr;
    std::list<std::shared_ptr<routine>> _routines;
    std::unordered_map<int, std::shared_ptr<routine>> _paused_routines;
    int _request_co_count = 0;
    struct event_base* _event_base = nullptr;
};

} // sco
