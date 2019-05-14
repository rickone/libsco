#include "sco_event.h"
#include "sco_except.h"
#include "sco_scheduler.h"

using namespace sco;

static void event_handler(evutil_socket_t fd, short flag, void* arg) {
#ifdef SCO_DEBUG
    printf("[SCO] on_event(%d, %d, %p)\n", fd, flag, arg);
#endif
    event_trigger* trigger = (event_trigger*)arg;
    trigger->on_event(fd, (int)flag);
}

struct event* sco::add_event(evutil_socket_t fd, int flag, int64_t timeout_usec, event_trigger* trigger) {
    auto scheduler = scheduler::current();
    runtime_assert(scheduler, "");

    auto base = scheduler->event_inst();
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
