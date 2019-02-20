#include "asyn_mutex.h"
#include "asyn_worker.h"
#include "asyn_panic.h"

using namespace asyn;

void mutex::lock() {
    auto cur_worker = worker::current();
    if (!cur_worker) {
        panic("!cur_worker");
    }

    auto self = cur_worker->co_self();
    if (!self) {
        panic("!self");
    }

    while (true) {
        int nil = 0;
        if (_cid.compare_exchange_weak(nil, self->id())) {
            return;
        }

        cur_worker->pause();
    }
}

void mutex::unlock() {
    auto self = coroutine::self();
    if (!self) {
        panic("!self");
    }

    int self_cid = self->id();
    if (!_cid.compare_exchange_weak(self_cid, 0)) {
        panic("!_cid.compare_exchange_weak(%d, 0)", self_cid);
    }
}
