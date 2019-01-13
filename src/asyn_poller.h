#pragma once

#include "asyn_coroutine.h"

namespace asyn {

const static int MAX_SELECT_COUNT = 1024;

enum {
    EVENT_NONE,
    EVENT_READ,
    EVENT_WRITE,
    EVENT_READ_WRITE,
};

class poller {
public:
    poller() = default;
    ~poller();

    void init();
    void add(int fd, int event_flag, coroutine* co);
    void set(int fd, int event_flag, coroutine* co);
    void remove(int fd);
    void poll(int64_t ns);
    void wait(int fd, int event_flag);

private:
    int _fd = -1;
};

} // asyn
