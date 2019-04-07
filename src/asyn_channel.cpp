#include "asyn_channel.h"
#include "asyn_worker.h"
#include "asyn_except.h"

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
    auto worker = worker::current();
    runtime_assert(worker, "");

    while (true) {
        box::object obj;
        if (_queue.pop(obj)) {
            return obj;
        }

        worker->pause();
    }
}

bool channel::wait_obj_until(const std::chrono::steady_clock::time_point& tp, box::object& obj) {
    auto worker = worker::current();
    runtime_assert(worker, "");

    auto timer = worker->timer_inst();
    timer->add_trigger(tp, this);

    _timeout = false;
    while (!_timeout) {
        if (_queue.pop(obj)) {
            timer->remove_trigger(tp, this);
            return true;
        }

        worker->pause();
    }

    return false;
}

void channel::on_timer() {
    _timeout = true;
}
