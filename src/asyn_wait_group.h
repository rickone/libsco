#pragma once

#include <functional>
#include <atomic>

namespace asyn {

class wait_group {
public:    
    wait_group() = default;
    ~wait_group() = default;
    wait_group(const wait_group&) = delete;
    wait_group(wait_group&&) = delete;
    wait_group& operator=(const wait_group&) = delete;

    void start(const std::function<void ()>& f);
    void start(void (*f)());
    void done();
    void wait();

private:
    std::atomic<int> _count;
};

} // asyn
