#pragma once

#include <unordered_map>
#include "asyn_coroutine.h"
#include "asyn_queue.h"
#include "asyn_worker.h"

namespace asyn {

enum {
    sch_coroutine_start,
    sch_coroutine_stop,
    sch_join,
};

struct co_status {
    int cid = 0;
    int wid = 0;
    int join_cid = 0;
};

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
    void on_coroutine_start(int cid, int wid);
    void on_coroutine_stop(int cid);
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

private:
    co_status* get_co_status(int cid);

private:
    int _code = 0;
    volatile bool _run_flag = false;
    queue<std::shared_ptr<coroutine>> _coroutines;
    worker _workers[4];
    queue<box::object> _requests;

    int _next_cid = 0;
    std::unordered_map<int, co_status> _co_status;
};

} // asyn
