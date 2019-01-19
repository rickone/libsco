#include "asyn_mutex.h"
#include "asyn_worker.h"

using namespace asyn;

mutex::mutex() {
    static std::atomic<int> s_next_id;
    _id = ++s_next_id;
}

mutex::mutex(const mutex& other) : _id(other._id) {
}

mutex::mutex(mutex&& other) : _id(other._id) {
    other._id = 0;
}

void mutex::lock() {
    if (_id == 0) { // panic
        return;
    }

    auto cur_worker = worker::current();
    if (!cur_worker) { // panic
        return;
    }

    cur_worker->lock(_id);
}

void mutex::unlock() {
    if (_id == 0) { // panic
        return;
    }

    auto cur_worker = worker::current();
    if (!cur_worker) { // panic
        return;
    }

    cur_worker->unlock(_id);
}
