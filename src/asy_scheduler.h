#pragma once

#include <vector>
#include <thread>
#include <pthread.h>
#include "asy_coroutine.h"
#include "asy_queue.h"

namespace asy {

class scheduler {
public:
    scheduler() = default;
    virtual ~scheduler() = default;

    static scheduler* inst();
    
    int run(int (*main)(int, char*[]), int argc, char* argv[]);
    void push_func(const coroutine::func_t& func);
    coroutine::func_t pop_func();
    void on_thread();
    
    void quit() { _run_flag = false; }

private:
    volatile bool _run_flag = false;
    queue<coroutine::func_t> _funcs;
    std::vector<std::thread> _threads;
};

} // asy
