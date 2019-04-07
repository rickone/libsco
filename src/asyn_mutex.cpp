#include "asyn_mutex.h"
#include "asyn_worker.h"
#include "asyn_except.h"

using namespace asyn;

void mutex::lock() {
    auto worker = worker::current();
    runtime_assert(worker, "");

    auto self = worker->co_self();
    runtime_assert(self, "");

    while (true) {
        int nil = 0;
        if (_cid.compare_exchange_weak(nil, self->id())) {
            return;
        }

        worker->pause();
    }
}

void mutex::unlock() {
    auto self = coroutine::self();
    runtime_assert(self, "");

    int self_cid = self->id();
    bool succ = _cid.compare_exchange_weak(self_cid, 0);
    runtime_assert(succ, "self_cid=%d", self_cid);
}
