#include "asyn_wait_group.h"
#include "asyn_master.h"

using namespace asyn;

void wait_group::add(int count) {
    _count += count;
}

void wait_group::done() {
    int count = --_count;
    if (count != 0) {
        return;
    }

    if (_cid == 0) {
        return;
    }

    master::inst()->command_worker(_wid, cmd_resume, _cid);
}

void wait_group::wait() {
    if (_count <= 0) { // panic
        return;
    }

    auto cur_worker = worker::current();
    if (!cur_worker) { // panic
        return;
    }

    auto self = cur_worker->co_self();
    if (!self) { // panic
        return;
    }

    _cid = self->id();
    _wid = cur_worker->id();
    cur_worker->yield(nullptr);
}
