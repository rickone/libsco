#pragma once

#include <vector>
#include <thread>
#include "asy_queue.h"

namespace asy {

struct uthread {
    void* (*func)(void*);
    void* arg;
};

class scheduler {
public:
    scheduler() = default;
    ~scheduler();

    void run();
    void add(decltype(uthread::func) func, void* arg);
    uthread* pop_uthread();

    void quit() { _run_flag = false; }

private:
    void on_thread();

    volatile bool _run_flag = false;
    queue _uthreads;
    std::vector<std::thread> _threads;
};

} // asy
