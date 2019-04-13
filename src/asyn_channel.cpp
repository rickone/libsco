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
    int64_t timeout_usec = std::chrono::duration_cast<std::chrono::microseconds>(tp - std::chrono::steady_clock::now()).count();
    if (timeout_usec <= 0) {
        return false;
    }

    auto worker = worker::current();
    runtime_assert(worker, "");

    auto timer = add_event(-1, 0, timeout_usec, this);
    runtime_assert(timer, "");

    _timeout = false;
    while (!_timeout) {
        if (_queue.pop(obj)) {
            evtimer_del(timer);
            return true;
        }

        worker->pause();
    }

    return false;
}

void channel::on_event(evutil_socket_t fd, int flag) {
    if (flag & EV_TIMEOUT) {
        _timeout = true;
    }
}
