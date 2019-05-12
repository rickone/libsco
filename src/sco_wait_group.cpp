#include "sco_wait_group.h"
#include "sco_except.h"

using namespace sco;

wait_group::wait_group() : _count(0) {
}

void wait_group::done() {
    _count.fetch_sub(1, std::memory_order_release);
}

void wait_group::wait() {
    auto worker = worker::current();
    runtime_assert(worker, "");

    while (true) {
        int count = _count.load(std::memory_order_consume);
        if (count == 0) {
            return;
        }

        worker->pause();
    }
}
