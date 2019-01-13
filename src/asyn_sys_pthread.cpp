#include "asyn_dlfunc.h"
#include <cstdio>
#include <pthread.h>
#include <signal.h>
#include <sys/wait.h>
#include "asyn_master.h"

using namespace asyn;

// part 1: pthread_t
sys_hook(pthread_create)
int pthread_create(pthread_t* thread, const pthread_attr_t* attr, void*(*start_routine)(void*), void* arg) {
    if (!worker::current()) {
        return sys_org(pthread_create)(thread, attr, start_routine, arg);
    }

    puts("asyn --> pthread_create");
    coroutine::func_t func = [start_routine, arg](){
        start_routine(arg);
    };
    int cid = master::inst()->start_coroutine(func);
    *thread = (pthread_t)(intptr_t)cid;
    return 0;
}

sys_hook(pthread_self)
pthread_t pthread_self() {
    auto w = worker::current();
    if (!w) {
        return sys_org(pthread_self)();
    }

    puts("asyn --> pthread_self");
    int cid = w->co_self()->id();
    return (pthread_t)(intptr_t)cid;
}

sys_hook(pthread_detach)
int pthread_detach(pthread_t thread) {
    if (!worker::current()) {
        return sys_org(pthread_detach)(thread);
    }

    puts("asyn --> pthread_detach");
    return 0;
}

sys_hook(pthread_join)
int pthread_join(pthread_t thread, void** retval) {
    if (!worker::current()) {
        return sys_org(pthread_join)(thread, retval);
    }

    puts("asyn --> pthread_join");
    int cid = (int)(intptr_t)thread;
    join(cid);
    return 0;
}

sys_hook(pthread_equal)
int pthread_equal(pthread_t thread1, pthread_t thread2) {
    return sys_org(pthread_equal)(thread1, thread2);
}

sys_hook(pthread_cancel)
int pthread_cancel(pthread_t thread) {
    return sys_org(pthread_cancel)(thread);
}

sys_hook(pthread_exit)
void pthread_exit(void* retval) {
    sys_org(pthread_exit)(retval);
}

sys_hook(pthread_kill)
int pthread_kill(pthread_t thread, int sig) {
    return sys_org(pthread_kill)(thread, sig);
}

sys_hook(pthread_getschedparam)
int pthread_getschedparam(pthread_t thread, int* policy, struct sched_param* param) {
    return sys_org(pthread_getschedparam)(thread, policy, param);
}

sys_hook(pthread_setschedparam)
int pthread_setschedparam(pthread_t thread, int policy, const struct sched_param* param) {
    return sys_org(pthread_setschedparam)(thread, policy, param);
}

// part 2: pthread_mutex_t
sys_hook(pthread_mutex_init)
int pthread_mutex_init(pthread_mutex_t* mutex, const pthread_mutexattr_t* attr) {
    return sys_org(pthread_mutex_init)(mutex, attr);
}

// part 3: pthread_cond_t
sys_hook(pthread_cond_wait)
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex) {
    return sys_org(pthread_cond_wait)(cond, mutex);
}

sys_hook(pthread_cond_timedwait)
int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime) {
    return sys_org(pthread_cond_timedwait)(cond, mutex, abstime);
}
