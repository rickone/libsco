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
    _max_co_count = MIN_CO_COUNT;
    _timeslice_ns = TIMESLICE_NANOSEC;
}

void worker::join() {
    pthread_join(_thread, nullptr);
}

void worker::pause() {
    if (!_self) {
        return;
    }

    auto ret = _paused_coroutines.emplace(_self->id(), _self->shared_from_this());
    if (!ret.second) {
        return;
    }

    _self->yield();
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
    auto tp_begin = std::chrono::steady_clock::now();

    auto it = _coroutines.begin();
    auto it_end = _coroutines.end();
    while (it != it_end) {
        auto& co = *it;
        if (co->status() == COROUTINE_DEAD) {
            _coroutines.erase(it++);
        } else {
            ++it;
        }
    }

    process_paused_coroutines();

    _timer.tick();

    int co_count = 0;
    for (; co_count < _max_co_count; co_count++) {
        auto co = mast->pop_coroutine();
        if (!co) {
            break;
        }

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

void worker::process_paused_coroutines() {
    auto it = _paused_coroutines.begin();
    auto it_end = _paused_coroutines.end();
    while (it != it_end) {
        auto co = it->second;
        _paused_coroutines.erase(it++);

        co->resume();
    }
}
