#include "sco_auto_scheduler.h"
#include <vector>
#include <unistd.h>
#include "sco_scheduler.h"
#include "sco_global_queue.h"

using namespace sco;

auto_scheduler::auto_scheduler(int scheduler_num, bool bind_cpu_core) : _main_co(std::bind(&auto_scheduler::main, this, scheduler_num, bind_cpu_core)) {
    auto co = global_queue::inst()->push_routine(nullptr);

    _main_co.init();
    co->swap(&_main_co);
}

void auto_scheduler::main(int scheduler_num, bool bind_cpu_core) {
    int cpu_num = (int)sysconf(_SC_NPROCESSORS_ONLN);
    if (scheduler_num <= 0) {
        scheduler_num = cpu_num;
    }

    std::vector<std::shared_ptr<scheduler>> schedulers;
    for (int i = 0; i < scheduler_num; i++) {
        schedulers.emplace_back(new scheduler());
    }

    for (int i = 1; i < scheduler_num; i++) {
        schedulers[i]->run_in_thread();
    }

    if (bind_cpu_core) {
        for (int i = 0; i < scheduler_num && i < cpu_num; i++) {
            schedulers[i]->bind_cpu_core(i);
        }
    }

    schedulers[0]->run(&_main_co);
}
