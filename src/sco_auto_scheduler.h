#pragma once

#include "sco_routine.h"

namespace sco {

class auto_scheduler {
public:
    explicit auto_scheduler(int scheduler_num = 0, bool bind_cpu_core = false);
    ~auto_scheduler() = default;

    void main(int scheduler_num, bool bind_cpu_core);

private:
    routine _main_co;
};

} // sco
