#include "asy_scheduler.h"
#include "asy_timer.h"
#include "asy_context.h"

using namespace asy;
using namespace std::chrono_literals;

scheduler::~scheduler() {
    for (auto& t : _threads) {
        t.join();
    }
}

scheduler* scheduler::inst() {
    static scheduler s_inst;
    return &s_inst;
}

void scheduler::run() {
    _run_flag = true;

    for (int i = 0; i < 4; ++i) {
        _threads.emplace_back(std::bind(&scheduler::on_thread, this));
    }
    on_thread();
}

void scheduler::push_func(const coroutine::func_t& func) {
    _funcs.push(func);
}

coroutine::func_t scheduler::pop_func() {
    coroutine::func_t func;
    _funcs.pop(func);
    return func;
}

void scheduler::on_thread() {
    coroutine co;
    timer ti;

    auto ctx = get_context();
    ctx->co = &co;
    ctx->ti = &ti;

    std::list<std::shared_ptr<coroutine>> coroutine_list;

    while (_run_flag) {
        //auto tp_begin = std::chrono::steady_clock::now();

        auto it = coroutine_list.begin();
        auto it_end = coroutine_list.end();
        while (it != it_end) {
            auto& co = *it;
            if (co->status() == COROUTINE_DEAD) {
                coroutine_list.erase(it++);
            } else {
                ++it;
            }
        }

        ti.tick();

        auto func = pop_func();
        if (func) {
            auto co = std::make_shared<coroutine>();
            co->bind(func);
            co->resume();
            coroutine_list.push_back(co);
        }

        std::this_thread::sleep_for(10ms);
    }
}
