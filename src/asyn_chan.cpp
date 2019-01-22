#include "asyn_chan.h"
#include "asyn_worker.h"

using namespace asyn;

chan::chan() {
    _queue = std::make_shared<lockfree_queue<box::object>>();
}

chan::~chan() {
    _queue.reset();
}

void chan::send_obj(const box::object& obj) {
    _queue->push(obj);
}

box::object chan::recv_obj() {
    auto cur_worker = worker::current();
    if (!cur_worker) { // panic
        return nullptr;
    }

    while (true) {
        box::object obj;
        if (_queue->pop(obj)) {
            return obj;
        }

        cur_worker->pause();
    }
}
