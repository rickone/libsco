#include "asyn_worker.h"
#include "asyn_master.h"
#include "asyn_coroutine.h"
#include "asyn_timer.h"
#include "asyn_poller.h"
#include "asyn_override.h"

ASYN_ORIGIN_DEF(pthread_create);
ASYN_ORIGIN_DEF(pthread_join);

using namespace asyn;
using namespace std::chrono_literals;

static pthread_key_t s_context_key;
static pthread_once_t s_context_once;

static void make_context_key() {
    pthread_key_create(&s_context_key, nullptr);
}

static void* start_routine(void* arg) {
    auto inst = (worker*)arg;
    inst->on_exec();
    return nullptr;
}

worker* worker::current() {
    return (worker*)pthread_getspecific(s_context_key);
}

void worker::run(int id) {
    _id = id;
    ASYN_ORIGIN(pthread_create)(&_thread, nullptr, start_routine, this);
}

void worker::init_context() {
    pthread_once(&s_context_once, make_context_key);
    pthread_setspecific(s_context_key, this);
}

void worker::join() {
    ASYN_ORIGIN(pthread_join)(_thread, nullptr);
}

void worker::yield(coroutine* co) {
    _yield_coroutines.insert(std::make_pair(co->id(), co->shared_from_this()));
    co->yield();
}

void worker::on_exec() {
    init_context();

    coroutine co;
    co.init();
    _self = &co;
    
    _poller.init();

    master* inst = master::inst();
    while (inst->is_running()) {
        //auto tp_begin = std::chrono::steady_clock::now();
        while (true) {
            box::object obj;
            if (!_requests.pop(obj)) {
                break;
            }

            auto type = obj.load<int>();
            on_command(type, obj);
        }
        
        auto it = _coroutines.begin();
        auto it_end = _coroutines.end();
        while (it != it_end) {
            auto& co = *it;
            int cid = co->id();
            if (co->status() == COROUTINE_DEAD) {
                _coroutines.erase(it++);

                inst->request(req_coroutine_stop, cid);
            } else {
                ++it;
            }
        }

        _timer.tick();

        auto co = inst->pop_coroutine();
        if (co) {
            inst->request(req_coroutine_start, co->id(), _id);

            co->init();
            co->resume();
            _coroutines.push_back(co);
        }

        _poller.wait(10'000'000);
    }
}

void worker::on_command(int type, box::object& obj) {
    switch (type) {
        case cmd_resume:
            obj.invoke(&worker::on_resume, this);
            break;
    }
}

void worker::on_resume(int cid) {
    auto it = _yield_coroutines.find(cid);
    if (it == _yield_coroutines.end()) {
        return;
    }

    std::shared_ptr<coroutine> co = it->second;
    _yield_coroutines.erase(it);
    co->resume();
}
