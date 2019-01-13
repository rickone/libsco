#pragma once

#include <unordered_map>
#include "asyn_coroutine.h"
#include "asyn_lockfree_queue.h"
#include "asyn_worker.h"
#include "asyn_monitor.h"

namespace asyn {

class master {
public:
    master() = default;
    ~master() = default;

    static master* inst();
    
    void enter();
    void quit(int code = 0);
    void main();
    int start_coroutine(const coroutine::func_t& func);
    std::shared_ptr<coroutine> pop_coroutine();
    void on_thread();
    void on_request(int type, box::object& obj);
    void on_join(int cid, int target_cid);

    bool is_startup() const { return _startup; }

    template<typename... A>
    void request(int type, A... args) {
        box::object obj;
        obj.store(type);
        obj.store_args(args...);
        _requests.push(std::move(obj));
    }

    template<typename... A>
    void command_worker(int wid, int type, A... args) {
        _workers[wid].command(type, args...);
    }

private:
    int _code = 0;
    volatile bool _startup = false;
    int _next_cid = 0;
    lockfree_queue<std::shared_ptr<coroutine>> _coroutines;
    worker _workers[5];
    lockfree_queue<box::object> _requests;
    monitor _monitor;
    coroutine _master_co;
};

enum { // request type
    req_coroutine_start,
    req_coroutine_stop,
    req_join,
};

class guard {
public:
    guard() {
        master::inst()->enter();
    }
    ~guard() {
        master::inst()->quit();
    }
};

inline int start(const coroutine::func_t& func) {
    return master::inst()->start_coroutine(func);
}

void join(int cid);

} // asyn
