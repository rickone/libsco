#pragma once

#include <functional>
#include <memory>
#include <atomic>

namespace asyn {

class wait_group {
public:    
    wait_group();
    ~wait_group() = default;
    wait_group(const wait_group& other);
    wait_group(wait_group&& other);

    void start(const std::function<void ()>& f);
    void start(void (*f)());
    void done();
    void wait();

private:
    std::shared_ptr<std::atomic<int>> _count;
};

} // asyn
