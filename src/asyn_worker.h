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

#define MIN_CO_COUNT 16
#define TIMESLICE_NANOSEC 10'000'000ll

class worker {
public:
    worker() = default;
    ~worker() = default;

    static worker* current();

    void run(int id);
    void init_thread(coroutine* self);
    void join();
    void pause();
    box::object resume(int cid, const box::object& obj);
    box::object yield(const box::object& obj);
    void yield_return(const box::object& obj);
    void lock(int mid);
    void unlock(int mid);
    void on_thread();
    void on_step();
    void process_paused_coroutines();
    void on_command(int type, box::object& obj);
    void on_resume(int cid, const std::string& str);

    template<typename... A>
    void command(int type, A... args) {
        box::object obj;
        obj.store(type);
        obj.store_args(args...);
        _commands.push(std::move(obj));
    }

    int id() const { return _id; }
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
    std::unordered_map<int, std::shared_ptr<coroutine>> _paused_coroutines;
    int _max_co_count = 0;
    int64_t _timeslice_ns = 0;
};

enum { // command type
    cmd_resume,
};

} // asyn
