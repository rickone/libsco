#pragma once

#include <vector>
#include <thread>
#include "asy_coroutine.h"
#include "asy_queue.h"

namespace asy {

class scheduler {
public:
    scheduler() = default;
    ~scheduler();

    static scheduler* inst();
    
    void run();
    void push_func(const coroutine::func_t& func);
    coroutine::func_t pop_func();

    void quit() { _run_flag = false; }

private:
    void on_thread();

    volatile bool _run_flag = false;
    queue<coroutine::func_t> _funcs;
    std::vector<std::thread> _threads;
};

} // asy
