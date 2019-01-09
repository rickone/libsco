#pragma once

#include "asy_coroutine.h"
#include "asy_queue.h"
#include "asy_executer.h"

namespace asy {

enum {
    sch_test,
};

class scheduler {
public:
    scheduler() = default;
    virtual ~scheduler() = default;

    static scheduler* inst();
    
    int run(int (*main)(int, char*[]), int argc, char* argv[]);
    void activate();
    std::shared_ptr<coroutine> start_coroutine(const coroutine::func_t& func);
    std::shared_ptr<coroutine> pop_coroutine();

    void on_request(int type, const xbin::object& obj);
    void on_test(int a, int b);

    bool is_running() const { return _run_flag; }
    void quit(int code) { _code = code; _run_flag = false; }

    template<typename... A>
    void request(int type, A... args) {
        xbin::object obj;
        obj.store(type);
        obj.store(args...);
        _requests.push(std::move(obj));
    }

private:
    int _code = 0;
    volatile bool _run_flag = false;
    queue<std::shared_ptr<coroutine>> _coroutines;
    executer _executers[4];
    queue<xbin::object> _requests;
};

} // asy
