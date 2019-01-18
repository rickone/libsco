#include "asyn_worker.h"
#include "asyn_master.h"
#include "asyn_coroutine.h"
#include "asyn_dlfunc.h"
#include "asyn_timer.h"
#include "asyn_poller.h"

using namespace asyn;
using namespace std::chrono_literals;

static pthread_key_t s_context_key;
static pthread_once_t s_context_once = PTHREAD_ONCE_INIT;

static void make_context_key() {
    pthread_key_create(&s_context_key, nullptr);
}

static void* work_routine(void* arg) {
    auto inst = (worker*)arg;
    inst->on_thread();
    return nullptr;
}

worker* worker::current() {
    pthread_once(&s_context_once, make_context_key);
    return (worker*)pthread_getspecific(s_context_key);
}

void worker::run(int id) {
    _id = id;
    pthread_create(&_thread, nullptr, work_routine, this);
}

void worker::init_thread(coroutine* self) {
    _self = self;
    _poller.init();
    pthread_once(&s_context_once, make_context_key);
    pthread_setspecific(s_context_key, this);
}

void worker::join() {
    pthread_join(_thread, nullptr);
}

void worker::yield(coroutine* co) {
    _yield_coroutines.insert(std::make_pair(co->id(), co->shared_from_this()));
    co->yield();
}

void worker::on_thread() {
    coroutine co;
    co.init();
    init_thread(&co);

    master* ma = master::inst();
    while (ma->is_startup()) {
        on_step();
    }
}

void worker::on_step() {
    auto ma = master::inst();
    //auto tp_begin = std::chrono::steady_clock::now();
    while (true) {
        box::object obj;
        if (!_commands.pop(obj)) {
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

            ma->request(req_coroutine_stop, cid);
        } else {
            ++it;
        }
    }

    _timer.tick();

    auto co = ma->pop_coroutine();
    if (co) {
        ma->request(req_coroutine_start, co->id(), _id);

        co->init();
        try {
            co->resume();    
        } catch (std::exception& err) {
            fprintf(stderr, "Error: %s\n", err.what());
        }
        
        _coroutines.push_back(co);
    }

    _poller.poll(10'000'000);
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
