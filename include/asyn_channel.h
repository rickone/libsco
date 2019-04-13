#pragma once

#include <chrono>
#include "box.h"
#include "asyn_lockfree_queue.h"
#include "asyn_event.h"

namespace asyn {

class channel : public event_trigger {
public:
    channel() = default;
    virtual ~channel() = default;
    channel(const channel&) = delete;
    channel(channel&&) = delete;
    channel& operator=(const channel&) = delete;

    void send_obj(const box::object& obj);
    box::object recv_obj();
    box::object wait_obj();
    bool wait_obj_until(const std::chrono::steady_clock::time_point& tp, box::object& obj);

    virtual void on_event(evutil_socket_t fd, int flag) override;

    template<typename T>
    void send(T val) {
        box::object obj;
        obj.store(val);
        send_obj(obj);
    }

    template<typename T>
    bool recv(T& val) {
        auto obj = recv_obj();
        if (!obj) {
            return false;
        }
        val = obj.load<T>();
        return true;
    }

    template<typename T>
    T wait() {
        auto obj = wait_obj();
        return obj.load<T>();
    }

    template <typename T>
    bool wait_until(const std::chrono::steady_clock::time_point& tp, T& val) {
        box::object obj;
        if (wait_obj_until(tp, obj)) {
            val = obj.load<T>();
            return true;
        }
        return false;
    }

    template <typename T, class Rep, class Period>
    bool wait_for(const std::chrono::duration<Rep, Period>& dtn, T& val) {
        auto tp = std::chrono::steady_clock::now() + dtn;
        return wait_until(tp, val);
    }

private:
    lockfree_queue<box::object> _queue;
    bool _timeout = false;
};

} // asyn
