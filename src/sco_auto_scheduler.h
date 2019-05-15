#pragma once

#include "sco_routine.h"

namespace sco {

class auto_scheduler {
public:
    explicit auto_scheduler(int scheduler_num = 0, bool bind_cpu_core = false);
    ~auto_scheduler() = default;
};

} // sco
