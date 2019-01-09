#include "asy_executer.h"
#include "asy_scheduler.h"
#include "asy_coroutine.h"
#include "asy_timer.h"
#include "asy_poller.h"
#include "asy_context.h"
#include "asy_override.h"

ASY_ORIGIN_DEF(pthread_create);
ASY_ORIGIN_DEF(pthread_join);

using namespace asy;
using namespace std::chrono_literals;

static void* start_routine(void* arg) {
    auto inst = (executer*)arg;
    inst->on_exec();
    return nullptr;
}

void executer::run(int id, scheduler* sche) {
    _id = id;
    _sche = sche;
    ASY_ORIGIN(pthread_create)(&_thread, nullptr, start_routine, this);
}

void executer::join() {
    ASY_ORIGIN(pthread_join)(_thread, nullptr);
}

void executer::on_exec() {
    coroutine self;
    self.init();

    timer timer;

    poller poller;
    poller.init();

    auto ctx = init_context();
    ctx->self = &self;
    ctx->timer = &timer;
    ctx->poller = &poller;

    while (_sche->is_running()) {
        //auto tp_begin = std::chrono::steady_clock::now();
        while (true) {
            box::object obj;
            if (!_requests.pop(obj)) {
                break;
            }

            auto type = obj.load<int>();
            on_request(type, obj);
        }
        
        auto it = _coroutine_list.begin();
        auto it_end = _coroutine_list.end();
        while (it != it_end) {
            auto& co = *it;
            if (co->status() == COROUTINE_DEAD) {
                _coroutine_list.erase(it++);
            } else {
                ++it;
            }
        }

        timer.tick();

        auto co = _sche->pop_coroutine();
        if (co) {
            co->init();
            co->resume();
            _coroutine_list.push_back(co);
        }

        poller.wait(10'000'000);
    }
}

void executer::on_request(int type, box::object& obj) {
    switch (type) {
        case sch_test:
            obj.invoke(&executer::on_test, this);
            break;
    }
}

void executer::on_test(int a, int b) {

}
