#include "asyn_wait_group.h"
#include "asyn_master.h"

using namespace asyn;

wait_group::wait_group() : _count(new std::atomic<int>(0)) {
}

wait_group::wait_group(const wait_group& other) : _count(other._count) {
}

wait_group::wait_group(wait_group&& other) : _count(other._count) {
}

void wait_group::start(const std::function<void ()>& f) {
    _count->fetch_add(1, std::memory_order_release);
    coroutine::func_t func = [f, this](){
        f();
        done();
    };
    master::inst()->start_coroutine(func);
}

void wait_group::start(void (*f)()) {
    _count->fetch_add(1, std::memory_order_release);
    coroutine::func_t func = [f, this](){
        f();
        done();
    };
    master::inst()->start_coroutine(func);
}

void wait_group::done() {
    _count->fetch_sub(1, std::memory_order_release);
}

void wait_group::wait() {
    auto cur_worker = worker::current();
    if (!cur_worker) { // panic
        return;
    }

    while (true) {
        int count = _count->load(std::memory_order_consume);
        if (count == 0) {
            return;
        }

        cur_worker->pause();
    }
}
