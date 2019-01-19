#pragma once

#include <atomic>
#include <utility>
#include "asyn_lockfree_queue.h"

namespace asyn {

class mutex {
public:    
    mutex() = default;
    mutex(const mutex& other) = delete;
    mutex(mutex&& other) = delete;
    ~mutex() = default;

    void lock();
    void unlock();

private:
    std::atomic<int> _cid;
    int _wid = -1;
    lockfree_queue<std::pair<int, int>> _wait_queue;
};

} // asyn
