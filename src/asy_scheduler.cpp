#include "asy_scheduler.h"
#include <unistd.h> // usleep
#include "asy_timer.h"
#include "asy_context.h"
#include "asy_override.h"

using namespace asy;
using namespace std::chrono_literals;

static void on_quit(int sig) {
    scheduler::inst()->quit();
}

scheduler* scheduler::inst() {
    static scheduler s_inst;
    return &s_inst;
}

int scheduler::run(int (*main)(int, char*[]), int argc, char* argv[]) {
    signal(SIGINT, on_quit); // ctrl + c
    signal(SIGTERM, on_quit); // kill
#if !defined(_WIN32)
    signal(SIGQUIT, on_quit); // ctrl + '\'
    signal(SIGCHLD, SIG_IGN);
#endif

    coroutine::func_t func = [this, main, argc, argv](){
        int ret = main(argc, argv);
        quit();
        //_quit_flag = true;
    };
    push_func(func);

    _run_flag = true;

    for (int i = 0; i < 4; ++i) {
        _threads.emplace_back(&scheduler::on_thread, this);
    }

    for (auto& thread : _threads) {
        thread.join();
    }
    return 0;
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
    co.init();
    timer ti;

    auto ctx = init_context();
    ctx->co = &co;
    ctx->ti = &ti;

    std::list<std::shared_ptr<coroutine>> coroutine_list;

    puts("on_thread start");

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
            co->init(func);
            co->resume();
            coroutine_list.push_back(co);
        }

        //usleep(10'000);
        std::this_thread::sleep_for(10ms);
    }

    puts("on_thread exit");
}
