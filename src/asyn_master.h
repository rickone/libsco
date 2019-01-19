#pragma once

#include <atomic>
#include "asyn_coroutine.h"
#include "asyn_lockfree_queue.h"
#include "asyn_worker.h"

namespace asyn {

class master {
public:
    master() = default;
    ~master() = default;

    static master* inst();
    
    void enter();
    void quit(int code = 0);
    void main();
    std::shared_ptr<coroutine> start_coroutine(const coroutine::func_t& func);
    std::shared_ptr<coroutine> pop_coroutine();
    void on_thread();

    bool is_startup() const { return _startup; }

    template<typename... A>
    void command_worker(int wid, int type, A... args) {
        if (wid < 0) {
            for (int i = 0; i < sizeof(_workers) / sizeof(_workers[0]); i++) {
                _workers[i].command(type, args...);
            }
            return;
        }

        if (wid < sizeof(_workers) / sizeof(_workers[0])) {
            _workers[wid].command(type, args...);
        }
    }

private:
    int _code = 0;
    std::atomic<bool> _startup;
    lockfree_queue<std::shared_ptr<coroutine>> _coroutines;
    worker _workers[5];
    coroutine _master_co;
};

} // asyn
