#include "asyn_master.h"
#include <unistd.h>
#include <signal.h>

using namespace asyn;

static void on_quit(int sig) {
    master::inst()->quit(1);
}

master* master::inst() {
    static master s_inst;
    return &s_inst;
}

void master::enter() {
    auto co = start_coroutine(nullptr);

    _master_co.init(std::bind(&master::main, this));
    co->swap(&_master_co);
}

void master::quit(int code) {
    _code = code;
    _startup = false;
    coroutine::self()->yield_return();
}

void master::main() {
    signal(SIGINT, on_quit); // ctrl + c
    signal(SIGTERM, on_quit); // kill
    signal(SIGQUIT, on_quit); // ctrl + '\'
    signal(SIGCHLD, SIG_IGN);

    _startup = true;
    for (int i = 1; i < 5; i++) {
        _workers[i].run(i);
    }

    on_thread();

    for (int i = 1; i < 5; i++) {
        _workers[i].join();
    }

    _exit(_code);
}

std::shared_ptr<coroutine> master::start_coroutine(const coroutine::func_t& func) {
    auto co = std::make_shared<coroutine>(func);
    if (!func) {
        co->init();
    }
    _coroutines.push(co);
    return co;
}

std::shared_ptr<coroutine> master::pop_coroutine() {
    std::shared_ptr<coroutine> co;
    _coroutines.pop(co);
    return co;
}

void master::on_thread() {
    _workers[0].init_thread(&_master_co);

    while (is_startup()) {
        _workers[0].on_step();
    }
}
