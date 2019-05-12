#pragma once

#include <atomic>
#include <utility>

namespace sco {

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

} // sco
