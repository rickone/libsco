#include "asyn_master.h"
#include <signal.h>
#include <time.h>
#include "asyn_override.h"

ASYN_ORIGIN_DEF(nanosleep);

using namespace asyn;

static void on_quit(int sig) {
    master::inst()->quit(1);
}

master* master::inst() {
    static master s_inst;
    return &s_inst;
}

int master::run(int (*main)(int, char*[]), int argc, char* argv[]) {
    signal(SIGINT, on_quit); // ctrl + c
    signal(SIGTERM, on_quit); // kill
    signal(SIGQUIT, on_quit); // ctrl + '\'
    signal(SIGCHLD, SIG_IGN);

    start_coroutine([this, main, argc, argv](){
        int ret = main(argc, argv);
        quit(ret);
    });

    _run_flag = true;
    int i = 0;
    for (auto& obj : _workers) {
        obj.run(i++);
    }

    on_exec();

    for (auto& obj : _workers) {
        obj.join();
    }
    return _code;
}

std::shared_ptr<coroutine> master::start_coroutine(const coroutine::func_t& func) {
    auto co = std::make_shared<coroutine>(++_next_cid, func);
    _coroutines.push(co);
    return co;
}

std::shared_ptr<coroutine> master::pop_coroutine() {
    std::shared_ptr<coroutine> co;
    _coroutines.pop(co);
    return co;
}

void master::on_exec() {
    while (_run_flag) {
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

        if (idle) {
            struct timespec req;
            req.tv_sec = 0;
            req.tv_nsec = 10'000'000;
            ASYN_ORIGIN(nanosleep)(&req, nullptr);
        }
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
