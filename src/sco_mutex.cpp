#include "sco_mutex.h"
#include "sco_scheduler.h"
#include "sco_except.h"

using namespace sco;

void mutex::lock() {
    auto scheduler = scheduler::current();
    runtime_assert(scheduler, "");

    auto self = scheduler->co_self();
    runtime_assert(self, "");

    while (true) {
        int nil = 0;
        if (_cid.compare_exchange_weak(nil, self->id())) {
            return;
        }

        scheduler->pause();
    }
}

void mutex::unlock() {
    auto self = routine::self();
    runtime_assert(self, "");

    int self_cid = self->id();
    bool succ = _cid.compare_exchange_weak(self_cid, 0);
    runtime_assert(succ, "self_cid=%d", self_cid);
}
