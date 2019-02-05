#pragma once

#include <unordered_map>
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
    void add(int fd, int event_flag);
    void set(int fd, int event_flag);
    void remove(int fd);
    void poll(int64_t ns);
    void wait(int fd, int event_flag);
    void resume_read(int fd);
    void resume_write(int fd);

private:
    int _fd = -1;
    std::unordered_map<int, std::shared_ptr<coroutine>> _read_wait_cos;
    std::unordered_map<int, std::shared_ptr<coroutine>> _write_wait_cos;
};

} // asyn
