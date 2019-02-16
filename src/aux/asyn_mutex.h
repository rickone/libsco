#pragma once

#include <atomic>
#include <utility>
#include "asyn_lockfree_queue.h"

namespace asyn {

class mutex {
public:    
    mutex() = default;
    ~mutex() = default;
    mutex(const mutex& other);
    mutex(mutex&& other);

    void lock();
    void unlock();

private:
    std::atomic<int> _cid;
};

} // asyn
