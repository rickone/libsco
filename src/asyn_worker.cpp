#include "asyn_worker.h"
#include "asyn_master.h"
#include "asyn_coordinator.h"
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
    _max_co_count = MIN_CO_COUNT;
    _timeslice_ns = TIMESLICE_NANOSEC;
}

void worker::join() {
    pthread_join(_thread, nullptr);
}

box::object worker::resume(int cid, const box::object& obj) {
    if (!_self) {
        return nullptr;
    }

    int self_cid = _self->id();
    if (self_cid == cid) {
        return nullptr;
    }

    _yield_coroutines.insert(std::make_pair(self_cid, _self->shared_from_this()));
    coordinator::inst()->request(req_resume, self_cid, cid, obj.str());
    _self->yield();
    return _self->get_value();
}

box::object worker::yield(const box::object& obj) {
    if (!_self) {
        return nullptr;
    }

    int self_cid = _self->id();
    _yield_coroutines.insert(std::make_pair(self_cid, _self->shared_from_this()));
    coordinator::inst()->request(req_yield, self_cid, obj.str());
    _self->yield();
    return _self->get_value();
}

void worker::yield_return(const box::object& obj) {
    if (!_self) {
        return;
    }

    int self_cid = _self->id();
    _yield_coroutines.insert(std::make_pair(self_cid, _self->shared_from_this()));
    coordinator::inst()->request(req_return, self_cid, obj.str());
    _self->yield_return();
}

void worker::lock(int mid) {
    if (!_self) {
        return;
    }

    int self_cid = _self->id();
    _yield_coroutines.insert(std::make_pair(self_cid, _self->shared_from_this()));
    coordinator::inst()->request(req_lock, self_cid, mid);
    _self->yield();
}

void worker::unlock(int mid) {
    if (!_self) {
        return;
    }

    int self_cid = _self->id();
    coordinator::inst()->request(req_unlock, self_cid, mid);
}

void worker::on_thread() {
    coroutine co;
    co.init();
    init_thread(&co);

    master* mast = master::inst();
    while (mast->is_startup()) {
        on_step();
    }
}

void worker::on_step() {
    auto mast = master::inst();
    auto coor = coordinator::inst();
    auto tp_begin = std::chrono::steady_clock::now();

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

            coor->request(req_return, cid);
        } else {
            ++it;
        }
    }

    _timer.tick();

    int co_count = 0;
    for (; co_count < _max_co_count; co_count++) {
        auto co = mast->pop_coroutine();
        if (!co) {
            break;
        }

        coor->request(req_start, co->id(), _id);

        co->init();
        try {
            co->resume();
        } catch (std::exception& err) {
            fprintf(stderr, "Error: %s\n", err.what());
        }
        
        _coroutines.push_back(co);
    }

    if (co_count == _max_co_count) {
        _max_co_count = _max_co_count * 4;
    } else {
        _max_co_count = std::max(MIN_CO_COUNT, _max_co_count / 2);
    }

    auto tp = std::chrono::steady_clock::now();
    int64_t ns = std::chrono::duration_cast<std::chrono::nanoseconds>(tp - tp_begin).count();
    int64_t remain_ns = 0;
    if (_timeslice_ns < ns) {
        _timeslice_ns += 1'000'000;
    } else {
        remain_ns = _timeslice_ns - ns;
        _timeslice_ns = std::max(TIMESLICE_NANOSEC, ns);
    }

    _poller.poll(remain_ns);
    //printf("co=%d, rn=%lld\n", _max_co_count, remain_ns);
}

void worker::on_command(int type, box::object& obj) {
    switch (type) {
        case cmd_resume:
            obj.invoke(&worker::on_resume, this);
            break;
    }
}

void worker::on_resume(int cid, const std::string& str) {
    auto it = _yield_coroutines.find(cid);
    if (it == _yield_coroutines.end()) {
        return;
    }

    std::shared_ptr<coroutine> co = it->second;
    _yield_coroutines.erase(it);

    box::object obj(str);
    co->move_value(std::move(obj));
    co->resume();
}
