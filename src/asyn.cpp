#include "asyn.h"

using namespace asyn;

guard g_guard_inst;

guard::guard() {
    master::inst()->enter();
}

guard::~guard() {
    master::inst()->quit();
}

void asyn::sleep_until(const std::chrono::steady_clock::time_point& tp) {
    auto worker = worker::current();
    runtime_assert(worker, "");

    auto self = worker->co_self();
    runtime_assert(self, "");

    worker->timer_inst()->add_trigger(tp, self);
    self->yield();
}
