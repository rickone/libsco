#include "asyn_override.h"
#include <cstdio>
#include <pthread.h>
#include "asyn_scheduler.h"

ASY_OVERRIDE(pthread_create)
int pthread_create(pthread_t* thread, const pthread_attr_t* attr, void*(*start_routine)(void*), void* arg) {
    puts("pthread_create hooked");
    return ASY_ORIGIN(pthread_create)(thread, attr, start_routine, arg);
}

ASY_OVERRIDE(pthread_self)
pthread_t pthread_self() {
    puts("pthread_self hooked");
    return ASY_ORIGIN(pthread_self)();
}

ASY_OVERRIDE(pthread_detach)
int pthread_detach(pthread_t thread) {
    puts("pthread_detach hooked");
    return ASY_ORIGIN(pthread_detach)(thread);
}

ASY_OVERRIDE(pthread_join)
int pthread_join(pthread_t thread, void** retval) {
    puts("pthread_join hooked");
    return ASY_ORIGIN(pthread_join)(thread, retval);
}

ASY_OVERRIDE(pthread_equal)
int pthread_equal(pthread_t thread1, pthread_t thread2) {
    puts("pthread_equal hooked");
    return ASY_ORIGIN(pthread_equal)(thread1, thread2);
}

ASY_OVERRIDE(pthread_cancel)
int pthread_cancel(pthread_t thread) {
    puts("pthread_cancel hooked");
    return ASY_ORIGIN(pthread_cancel)(thread);
}

ASY_OVERRIDE(pthread_exit)
void pthread_exit(void* retval) {
    puts("pthread_exit hooked");
    ASY_ORIGIN(pthread_exit)(retval);
}

ASY_OVERRIDE(pthread_getschedparam)
int pthread_getschedparam(pthread_t thread, int* policy, struct sched_param* param) {
    puts("pthread_getschedparam hooked");
    return ASY_ORIGIN(pthread_getschedparam)(thread, policy, param);
}

ASY_OVERRIDE(pthread_setschedparam)
int pthread_setschedparam(pthread_t thread, int policy, const struct sched_param* param) {
    puts("pthread_setschedparam hooked");
    return ASY_ORIGIN(pthread_setschedparam)(thread, policy, param);
}
