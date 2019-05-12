#pragma once

#include <chrono>
#include "sco_lockfree_queue.h"
#include "sco_event.h"
#include "sco_worker.h"
#include "sco_except.h"

namespace sco {

template<typename T>
class channel : public event_trigger {
public:
    channel() = default;
    virtual ~channel() = default;
    channel(const channel&) = delete;
    channel(channel&&) = delete;
    channel& operator=(const channel&) = delete;

    void send(const T& val) {
        _queue.send(val);
    }

    bool recv(T& val) {
        return _queue.pop(val);
    }

    void wait(T& val) {
        auto worker = worker::current();
        runtime_assert(worker, "");

        while (true) {
            if (recv(val)) {
                return val;
            }

            worker->pause();
        }
    }

    bool wait_until(const std::chrono::steady_clock::time_point& tp, T& val) {
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
            if (recv(val)) {
                evtimer_del(timer);
                return true;
            }

            worker->pause();
        }

        return false;
    }

    template<class Rep, class Period>
    bool wait_for(const std::chrono::duration<Rep, Period>& dtn, T& val) {
        auto tp = std::chrono::steady_clock::now() + dtn;
        return wait_until(tp, val);
    }

    virtual void on_event(evutil_socket_t fd, int flag) override {
        if (flag & EV_TIMEOUT) {
            _timeout = true;
        }
    }

private:
    lockfree_queue<T> _queue;
    bool _timeout = false;
};

} // sco
