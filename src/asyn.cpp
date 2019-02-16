#include "asyn.h"

using namespace asyn;

guard g_guard_inst;

guard::guard() {
    master::inst()->enter();
}

guard::~guard() {
    master::inst()->quit();
}

void asyn::nsleep(int64_t ns) {
    auto worker = worker::current();
    auto self = worker->co_self();
    worker->timer_inst()->add_trigger(ns, self);

#ifdef ASYN_DEBUG
    printf("[ASYN] coroutine(%d) nsleep, ns=%lld\n", self->id(), ns);
#endif
    self->yield();
}
