#pragma once

#include <functional>
#include <atomic>
#include "sco_master.h"

namespace sco {

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
        routine::func_t func = [f, this](){
            f();
            done();
        };
        master::inst()->start_routine(func);
    }

private:
    std::atomic<int> _count;
};

} // sco
