#pragma once

#include "asyn_coroutine.h"
#include "asyn_queue.h"
#include "asyn_executer.h"

namespace asyn {

enum {
    sch_test,
};

class scheduler {
public:
    scheduler() = default;
    virtual ~scheduler() = default;

    static scheduler* inst();
    
    int run(int (*main)(int, char*[]), int argc, char* argv[]);
    std::shared_ptr<coroutine> start_coroutine(const coroutine::func_t& func);
    std::shared_ptr<coroutine> pop_coroutine();
    void on_exec();
    void on_request(int type, box::object& obj);
    void on_test(int a, int b);

    bool is_running() const { return _run_flag; }
    void quit(int code) { _code = code; _run_flag = false; }

    template<typename... A>
    void request(int type, A... args) {
        box::object obj;
        obj.store(type);
        obj.store(args...);
        _requests.push(std::move(obj));
    }

private:
    int _code = 0;
    volatile bool _run_flag = false;
    queue<std::shared_ptr<coroutine>> _coroutines;
    executer _executers[4];
    queue<box::object> _requests;
};

} // asyn
