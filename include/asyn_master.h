#pragma once

#include <atomic>
#include "asyn_coroutine.h"
#include "asyn_lockfree_queue.h"
#include "asyn_worker.h"

namespace asyn {

class master {
public:
    master();
    ~master() = default;

    static master* inst();
    
    void enter();
    void quit(int code = 0);
    void main();
    std::shared_ptr<coroutine> start_coroutine(const coroutine::func_t& func);
    std::shared_ptr<coroutine> pop_coroutine();

    bool is_startup() const { return _startup; }

private:
    int _code = 0;
    std::atomic<bool> _startup;
    lockfree_queue<std::shared_ptr<coroutine>> _coroutines;
    coroutine _master_co;
};

} // asyn
