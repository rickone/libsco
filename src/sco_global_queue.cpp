#include "sco_global_queue.h"

using namespace sco;

global_queue* global_queue::inst() {
    static global_queue s_inst;
    return &s_inst;
}

std::shared_ptr<routine> global_queue::push_routine(const routine::func_t& func) {
    auto co = std::make_shared<routine>();
    co->bind(func);
    _routines.push(co);
    return co;
}

std::shared_ptr<routine> global_queue::pop_routine() {
    std::shared_ptr<routine> co;
    _routines.pop(co);
    return co;
}
