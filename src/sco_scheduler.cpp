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
#include "sco_scheduler.h"
#include "sco_global_queue.h"
#include "sco_routine.h"
#include "sco_event.h"
#include "sco_except.h"

using namespace sco;

static pthread_key_t s_context_key;
static pthread_once_t s_context_once = PTHREAD_ONCE_INIT;

static void make_context_key() {
    pthread_key_create(&s_context_key, nullptr);
}

static void* work_routine(void* arg) {
    ((scheduler*)arg)->run();
    return nullptr;
}

scheduler::~scheduler() {
    if (_event_base) {
        event_base_free(_event_base);
        _event_base = nullptr;
    }
}

scheduler* scheduler::current() {
    pthread_once(&s_context_once, make_context_key);
    return (scheduler*)pthread_getspecific(s_context_key);
}

void scheduler::run(routine* self) {
    std::shared_ptr<routine> co;

    if (!self) {
        co = std::make_shared<routine>(nullptr);
        co->init();
        self = co.get();
    }
    _self = self;
    _request_co_count = REQUEST_CO_COUNT;

    pthread_once(&s_context_once, make_context_key);
    pthread_setspecific(s_context_key, this);

    _event_base = event_base_new();
    runtime_assert(_event_base, "");

#ifdef SCO_DEBUG
    auto event_method = event_base_get_method(_event_base);
    printf("[SCO] scheduler(%p) event_method: %s\n", this, event_method);
#endif

    add_event(-1, EV_PERSIST, 10'000, this);

    event_base_dispatch(_event_base);

    printf("[SCO] event_base_dispatch exit!\n");
}

void scheduler::run_in_thread() {
    pthread_create(&_thread, nullptr, work_routine, this);
}

void scheduler::join() {
    pthread_join(_thread, nullptr);
}

void scheduler::bind_cpu_core(int cpu_core) {
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

void scheduler::pause() {
    if (!_self) {
        return;
    }

    auto ret = _paused_routines.emplace(_self->id(), _self->shared_from_this());
    if (!ret.second) {
        return;
    }

#ifdef SCO_DEBUG
    printf("[SCO] routine(%d) pause\n", _self->id());
#endif
    _self->yield();
}

void scheduler::on_event(evutil_socket_t fd, int flag) {
    process_new_routines();
    process_dead_routines();
    process_paused_routines();
}

void scheduler::process_new_routines() {
    auto q = global_queue::inst();
    int count = 0;
    for (; count < _request_co_count; count++) {
        auto co = q->pop_routine();
        if (!co) {
            break;
        }

        try {
            co->resume();
        } catch (std::exception& err) {
            fprintf(stderr, "[SCO] routine(%d) resume error: %s\n", co->id(), err.what());
        }
        
        _routines.push_back(co);
    }

    if (count == _request_co_count) {
        _request_co_count = _request_co_count * 2;
    } else {
        _request_co_count = std::max(REQUEST_CO_COUNT, _request_co_count / 2);
    }
}

void scheduler::process_dead_routines() {
    auto it = _routines.begin();
    auto it_end = _routines.end();
    while (it != it_end) {
        auto& co = *it;
        if (co->status() == COROUTINE_DEAD) {
            _routines.erase(it++);
        } else {
            ++it;
        }
    }
}

void scheduler::process_paused_routines() {
    auto it = _paused_routines.begin();
    auto it_end = _paused_routines.end();
    while (it != it_end) {
        auto co = it->second;
        _paused_routines.erase(it++);

        co->resume();
    }
}
