#include "asyn_event.h"
#include "asyn_except.h"
#include "asyn_worker.h"

using namespace asyn;

static void event_handler(evutil_socket_t fd, short flag, void* arg) {
#ifdef ASYN_DEBUG
    printf("[ASYN] on_event(%d, %d, %p)\n", fd, flag, arg);
#endif
    event_trigger* trigger = (event_trigger*)arg;
    trigger->on_event(fd, (int)flag);
}

struct event* asyn::add_event(evutil_socket_t fd, int flag, int64_t timeout_usec, event_trigger* trigger) {
    auto worker = worker::current();
    runtime_assert(worker, "");

    auto base = worker->event_inst();
    runtime_assert(base, "");

    auto event = event_new(base, fd, (short)flag, event_handler, trigger);
    runtime_assert(event, "");

    struct timeval tv;
    struct timeval* timeout = nullptr;
    if (timeout_usec > 0) {
        tv.tv_sec = (long)(timeout_usec / 1'000'000);
        tv.tv_usec = (long)(timeout_usec % 1'000'000);
        timeout = &tv;
    }

    int ret = event_add(event, timeout);
    runtime_assert(ret == 0, "");

    return event;
}

int asyn::wait_event(evutil_socket_t fd, int flag, int64_t timeout_usec) {
    auto worker = worker::current();
    runtime_assert(worker, "");

    auto co = worker->co_self();
    runtime_assert(co, "");

    add_event(fd, flag, timeout_usec, co);
    co->yield();

    return co->get_value().load<int>();
}
