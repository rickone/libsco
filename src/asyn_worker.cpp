#ifdef __linux__
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <pthread.h>
#endif
#ifdef __APPLE__
#define _DARWIN_C_SOURCE
#include <mach/thread_policy.h>
#include <mach/thread_act.h>
#include <pthread.h>
#endif
#include "asyn_worker.h"
#include "asyn_master.h"
#include "asyn_coroutine.h"
#include "asyn_event.h"
#include "asyn_except.h"

using namespace asyn;

static pthread_key_t s_context_key;
static pthread_once_t s_context_once = PTHREAD_ONCE_INIT;

static void make_context_key() {
    pthread_key_create(&s_context_key, nullptr);
}

static void* work_routine(void* arg) {
    ((worker*)arg)->run();
    return nullptr;
}

worker::~worker() {
    if (_event_base) {
        event_base_free(_event_base);
        _event_base = nullptr;
    }
}

worker* worker::current() {
    pthread_once(&s_context_once, make_context_key);
    return (worker*)pthread_getspecific(s_context_key);
}

void worker::run(coroutine* self) {
    std::shared_ptr<coroutine> co;

    if (!self) {
        co = std::make_shared<coroutine>(nullptr);
        co->init();
        self = co.get();
    }
    _self = self;
    _request_co_count = REQUEST_CO_COUNT;

    pthread_once(&s_context_once, make_context_key);
    pthread_setspecific(s_context_key, this);

    _event_base = event_base_new();
    runtime_assert(_event_base, "");

#ifdef ASYN_DEBUG
    auto event_method = event_base_get_method(_event_base);
    printf("[ASYN] worker(%p) event_method: %s\n", this, event_method);
#endif

    add_event(-1, EV_PERSIST, 10'000, this);

    event_base_dispatch(_event_base);
}

void worker::run_in_thread() {
    pthread_create(&_thread, nullptr, work_routine, this);
}

void worker::join() {
    pthread_join(_thread, nullptr);
}

void worker::bind_cpu_core(int cpu_core) {
    if (cpu_core < 0) {
        return;
    }

    if (!_thread) {
        _thread = pthread_self();
    }

#ifdef __linux__
    cpu_set_t cpu_info;
    CPU_ZERO(&cpu_info);
    CPU_SET(cpu_core, &cpu_info);

    int ret = pthread_setaffinity_np(_thread, sizeof(cpu_set_t), &cpu_info);
    runtime_assert_std(ret == 0, "pthread_setaffinity_np");
#endif // __linux__

#ifdef __APPLE__
    thread_affinity_policy_data_t policy = { cpu_core };
    thread_port_t mach_thread = pthread_mach_thread_np(_thread);
    int ret = thread_policy_set(mach_thread, THREAD_AFFINITY_POLICY, (thread_policy_t)&policy, 1);
    runtime_assert_std(ret == 0, "thread_policy_set");
#endif // __APPLE__
}

void worker::pause() {
    if (!_self) {
        return;
    }

    auto ret = _paused_coroutines.emplace(_self->id(), _self->shared_from_this());
    if (!ret.second) {
        return;
    }

#ifdef ASYN_DEBUG
    printf("[ASYN] coroutine(%d) pause\n", _self->id());
#endif
    _self->yield();
}

void worker::on_event(evutil_socket_t fd, int flag) {
    process_new_coroutines();
    process_dead_coroutines();
    process_paused_coroutines();
}

void worker::process_new_coroutines() {
    auto master = master::inst();
    int count = 0;
    for (; count < _request_co_count; count++) {
        auto co = master->pop_coroutine();
        if (!co) {
            break;
        }

        try {
            co->resume();
        } catch (std::exception& err) {
            fprintf(stderr, "[ASYN] coroutine(%d) resume error: %s\n", co->id(), err.what());
        }
        
        _coroutines.push_back(co);
    }

    if (count == _request_co_count) {
        _request_co_count = _request_co_count * 2;
    } else {
        _request_co_count = std::max(REQUEST_CO_COUNT, _request_co_count / 2);
    }
}

void worker::process_dead_coroutines() {
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
