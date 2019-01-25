#pragma once

#include "box.h"
#include "asyn_lockfree_queue.h"

namespace asyn {

class chan {
public:
    chan() = default;
    ~chan() = default;
    chan(const chan&) = delete;
    chan(chan&&) = delete;
    chan& operator=(const chan&) = delete;

    void send_obj(const box::object& obj);
    box::object recv_obj();
    box::object wait_obj();

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

private:
    lockfree_queue<box::object> _queue;
};

} // asyn
