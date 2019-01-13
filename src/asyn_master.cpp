#include "asyn_master.h"
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
    int cid = ++_next_cid;
    auto main_co = std::make_shared<coroutine>(cid, nullptr);
    main_co->init();
    _coroutines.push(main_co);

    _master_co.init(std::bind(&master::main, this));
    main_co->swap(&_master_co);
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
    for (int i = 1; i < 5; ++i) {
        _workers[i].run(i);
    }

    on_thread();

    for (int i = 1; i < 5; ++i) {
        _workers[i].join();
    }
    _exit(_code);
}

int master::start_coroutine(const coroutine::func_t& func) {
    int cid = ++_next_cid;
    auto co = std::make_shared<coroutine>(cid, func);
    _coroutines.push(co);
    return cid;
}

std::shared_ptr<coroutine> master::pop_coroutine() {
    std::shared_ptr<coroutine> co;
    _coroutines.pop(co);
    return co;
}

void master::on_thread() {
    _workers[0].init_thread(&_master_co);

    while (is_startup()) {
        bool idle = true;
        while (true) {
            box::object obj;
            if (!_requests.pop(obj)) {
                break;
            }

            idle = false;
            auto type = obj.load<int>();
            on_request(type, obj);
        }

        _workers[0].on_step();
    }
}

void master::on_request(int type, box::object& obj) {
    switch (type) {
        case req_coroutine_start:
            obj.invoke(&monitor::on_coroutine_start, &_monitor);
            break;
        case req_coroutine_stop:
            obj.invoke(&monitor::on_coroutine_stop, &_monitor);
            break;
        case req_join:
            obj.invoke(&master::on_join, this);
    }
}

void master::on_join(int cid, int target_cid) {
    auto status = _monitor.get_co_status(target_cid);
    if (status == nullptr) {
        auto join_status = _monitor.get_co_status(cid);
        if (join_status) {
            command_worker(join_status->wid, cmd_resume, cid);
        }
        return;
    }

    status->join_cid = cid;
}

void asyn::join(int cid) {
    auto self = coroutine::self();
    if (self == nullptr || self->id() == cid) {
        return;
    }

    master::inst()->request(req_join, self->id(), cid);
    worker::current()->yield(self);
}
