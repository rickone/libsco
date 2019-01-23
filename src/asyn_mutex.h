#pragma once

#include <memory>
#include <atomic>
#include <utility>
#include "asyn_lockfree_queue.h"

namespace asyn {

class mutex {
public:    
    mutex();
    ~mutex() = default;
    mutex(const mutex& other);
    mutex(mutex&& other);

    void lock();
    void unlock();

private:
    std::shared_ptr<std::atomic<int>> _cid;
};

} // asyn
