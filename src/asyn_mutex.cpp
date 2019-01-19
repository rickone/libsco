#include "asyn_mutex.h"
#include "asyn_worker.h"

using namespace asyn;

void mutex::lock() {
    auto cur_worker = worker::current();
    if (!cur_worker) { // panic
        return;
    }

    auto self = cur_worker->co_self();
    if (!self) { // panic
        return;
    }

    //compare_and_set _cid 0 -> self->id()
    // if success lock else yield
}

void mutex::unlock() {
    auto cur_worker = worker::current();
    if (!cur_worker) { // panic
        return;
    }

    auto self = cur_worker->co_self();
    if (!self) { // panic
        return;
    }

    // compare_and_set _cid self->id() -> 0
    // if success unlock else yield
}
