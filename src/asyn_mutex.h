#pragma once

namespace asyn {

class mutex {
public:    
    mutex();
    mutex(const mutex& other);
    mutex(mutex&& other);
    ~mutex() = default;

    void lock();
    void unlock();

private:
    int _id = 0;
};

} // asyn
