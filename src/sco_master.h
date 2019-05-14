#pragma once

#include <atomic>
#include "sco_routine.h"
#include "sco_lockfree_queue.h"
#include "sco_scheduler.h"

namespace sco {

class master {
public:
    master();
    ~master() = default;

    static master* inst();
    
    void enter();
    void quit(int code = 0);
    void main();
    std::shared_ptr<routine> start_routine(const routine::func_t& func);
    std::shared_ptr<routine> pop_routine();

    bool is_startup() const { return _startup; }

private:
    int _code = 0;
    std::atomic<bool> _startup;
    lockfree_queue<std::shared_ptr<routine>> _routines;
    routine _master_co;
};

} // sco
