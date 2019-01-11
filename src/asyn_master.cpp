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
        obj.run(i++, this);
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
        case sch_coroutine_start:
            obj.invoke(&master::on_coroutine_start, this);
            break;
        case sch_coroutine_stop:
            obj.invoke(&master::on_coroutine_stop, this);
            break;
        case sch_join:
            obj.invoke(&master::on_join, this);
    }
}

void master::on_coroutine_start(int cid, int wid) {
    co_status status = {cid, wid};
    _co_status[cid] = status;
}

void master::on_coroutine_stop(int cid) {
    auto status = get_co_status(cid);
    if (status == nullptr) {
        return;
    }

    if (status->join_cid != 0) {
        auto join_status = get_co_status(status->join_cid);
        if (join_status) {
            _workers[join_status->wid].request(exe_resume, status->join_cid);
        }
    }

    _co_status.erase(cid);
}

void master::on_join(int cid, int target_cid) {
    auto status = get_co_status(target_cid);
    if (status == nullptr) {
        auto join_status = get_co_status(cid);
        if (join_status) {
            _workers[join_status->wid].request(exe_resume, cid);
        }
        return;
    }

    status->join_cid = cid;
}

co_status* master::get_co_status(int cid) {
    auto it = _co_status.find(cid);
    if (it == _co_status.end()) {
        return nullptr;
    }
    return &it->second;
}
