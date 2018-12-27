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

void scheduler::run() {
    _run_flag = true;

    for (int i = 0; i < 4; ++i) {
        _threads.emplace_back(std::bind(&scheduler::on_thread, this));
    }
    on_thread();
}

void scheduler::add(decltype(uthread::func) func, void* arg) {
    uthread ut = {.func = func, .arg = arg};
    _uthreads.push((const char*)&ut, sizeof(ut));
}

uthread* scheduler::pop_uthread() {
    auto node = _uthreads.pop();
    if (node == nullptr) {
        return nullptr;
    }

    return (uthread*)node->data;
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

        auto ut = pop_uthread();
        if (ut) {
            auto co = std::make_shared<coroutine>();
            std::function<void(void)> func = [ut](){
                ut->func(ut->arg);
            };
            co->bind(func);
            co->resume();
            coroutine_list.push_back(co);
        }

        std::this_thread::sleep_for(10ms);
    }
}
