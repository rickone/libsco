#pragma once

#include "asy_coroutine.h"

namespace asy {

const static int MAX_SELECT_COUNT = 1024;

enum {
    SELECT_NONE,
    SELECT_READ,
    SELECT_WRITE,
    SELECT_READ_WRITE,
};

class poller {
public:
    poller() = default;
    virtual ~poller();

    void init();
    void add(int fd, int event_flag, coroutine* co);
    void set(int fd, int event_flag, coroutine* co);
    void remove(int fd);
    void wait(int64_t ns);

private:
    int _fd = -1;
};

} // asy
