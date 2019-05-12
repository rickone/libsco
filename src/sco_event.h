#pragma once

#include "event2/event.h"

namespace sco {

struct event_trigger {
    virtual void on_event(evutil_socket_t fd, int flag) = 0;
};

struct event* add_event(evutil_socket_t fd, int flag, int64_t timeout_usec, event_trigger* trigger);
int wait_event(evutil_socket_t fd, int flag, int64_t timeout_usec);

} // sco
