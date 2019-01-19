#pragma once

#include <atomic>

namespace asyn {

class wait_group {
public:    
    wait_group() = default;
    wait_group(const wait_group& other) = delete;
    wait_group(wait_group&& other) = delete;
    ~wait_group() = default;

    void add(int count);
    void done();
    void wait();

private:
    int _cid = 0;
    int _wid = -1;
    std::atomic<int> _count;
};

} // asyn
