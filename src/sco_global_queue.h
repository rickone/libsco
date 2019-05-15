#pragma once

#include <memory>
#include "sco_routine.h"
#include "sco_lockfree_queue.h"

namespace sco {

class global_queue {
public:
    global_queue() = default;
    ~global_queue() = default;

    static global_queue* inst();
    
    std::shared_ptr<routine> push_routine(const routine::func_t& func);
    std::shared_ptr<routine> pop_routine();

private:
    lockfree_queue<std::shared_ptr<routine>> _routines;
};

} // sco
