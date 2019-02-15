#include "asyn_channel.h"
#include "asyn_worker.h"
#include "asyn_panic.h"

using namespace asyn;

void channel::send_obj(const box::object& obj) {
    _queue.push(obj);
}

box::object channel::recv_obj() {
    box::object obj;
    _queue.pop(obj);
    return obj;
}

box::object channel::wait_obj() {
    auto cur_worker = worker::current();
    if (!cur_worker) {
        panic("!cur_worker");
    }

    while (true) {
        box::object obj;
        if (_queue.pop(obj)) {
            return obj;
        }

        cur_worker->pause();
    }
}
