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
    virtual ~master() = default;

    static master* inst();
    
    int run(int (*main)(int, char*[]), int argc, char* argv[]);
    std::shared_ptr<coroutine> start_coroutine(const coroutine::func_t& func);
    std::shared_ptr<coroutine> pop_coroutine();
    void on_exec();
    void on_request(int type, box::object& obj);
    void on_join(int cid, int target_cid);

    bool is_running() const { return _run_flag; }
    void quit(int code) { _code = code; _run_flag = false; }

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
    volatile bool _run_flag = false;
    int _next_cid = 0;
    lockfree_queue<std::shared_ptr<coroutine>> _coroutines;
    worker _workers[4];
    lockfree_queue<box::object> _requests;
    monitor _monitor;
};

enum { // request type
    req_coroutine_start,
    req_coroutine_stop,
    req_join,
};

} // asyn
