#pragma once

#include <functional>
#include <atomic>
#include "asyn_master.h"

namespace asyn {

class wait_group {
public:    
    wait_group();
    ~wait_group() = default;
    wait_group(const wait_group&) = delete;
    wait_group(wait_group&&) = delete;
    wait_group& operator=(const wait_group&) = delete;
    wait_group& operator=(wait_group&&) = delete;

    void done();
    void wait();

    template<typename F>
    void start(const F& f) {
        _count.fetch_add(1, std::memory_order_release);
        coroutine::func_t func = [f, this](){
            f();
            done();
        };
        master::inst()->start_coroutine(func);
    }

private:
    std::atomic<int> _count;
};

} // asyn
