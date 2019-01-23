#pragma once

#include "box.h"
#include "asyn_lockfree_queue.h"

namespace asyn {

class chan {
public:
    chan();
    ~chan() = default;
    chan(const chan& other);
    chan(chan&& other);

    void send_obj(const box::object& obj);
    box::object recv_obj();

    template<typename T>
    void send(T val) {
        box::object obj;
        obj.store(val);
        send_obj(obj);
    }

    template<typename T>
    T recv() {
        auto obj = recv_obj();
        return obj.load<T>();
    }

private:
    std::shared_ptr<lockfree_queue<box::object>> _queue;
};

} // asyn
