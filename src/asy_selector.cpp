#include "asy_selector.h"
#include <cerrno>

using namespace asy;

#ifdef __linux__
#include <sys/epoll.h>

selector::~selector() {
    if (_fd >= 0) {
        close(_fd);
    }
}

void selector::init() {
    _fd = epoll_create(1024);
}

void selector::add(int fd, int event_flag, coroutine* co) {
    struct epoll_event event;
    event.events = EPOLLET;
    event.data.ptr = co;

    if (event_flag & SELECT_READ) {
        event.events |= EPOLLIN;
    }

    if (event_flag & SELECT_WRITE) {
        event.events |= EPOLLOUT;
    }

    epoll_ctl(_fd, EPOLL_CTL_ADD, fd, &event);
}

void selector::set(int fd, int event_flag, coroutine* co) {
    struct epoll_event event;
    event.events = EPOLLET;
    event.data.ptr = socket;

    if (event_flag & SELECT_READ) {
        event.events |= EPOLLIN;
    }

    if (event_flag & SELECT_WRITE) {
        event.events |= EPOLLOUT;
    }

    epoll_ctl(_fd, EPOLL_CTL_MOD, fd, &event);
}

void selector::remove(int fd) {
    epoll_ctl(_fd, EPOLL_CTL_DEL, fd, nullptr);
}

void selector::select(int64_t ns) {
    static struct epoll_event events[MAX_SELECT_COUNT];
    
    int event_cnt = epoll_wait(_fd, events, MAX_SELECT_COUNT, (int)(ns / 1'000'000));
    if (event_cnt < 0) {
        if (errno == EINTR)
            return;

        perror("epoll_wait");
        return;
    }

    for (int i = 0; i < event_cnt; ++i) {
        unsigned es = events[i].events;
        int event_type = 0;
        if (es & EPOLLIN) {
            event_type |= SELECT_READ;
        }
        if (es & EPOLLOUT) {
            event_type |= SELECT_WRITE;
        }
        auto co = (coroutine*)events[i].data.ptr;
        co->set_value(event_type);
        co->resume();
    }
}
#endif // __linux__

#ifdef __APPLE__
#include <sys/event.h>

selector::~selector() {
    if (_fd >= 0) {
        close(_fd);
    }
}

void selector::init() {
    _fd = kqueue();
}

void selector::add(int fd, int event_flag, coroutine* co) {
    struct kevent events[2];
    EV_SET(&events[0], fd, EVFILT_READ,  EV_ADD | ((event_flag & SELECT_READ) ? EV_ENABLE : EV_DISABLE), 0, 0, co);
    EV_SET(&events[1], fd, EVFILT_WRITE, EV_ADD | ((event_flag & SELECT_WRITE) ? EV_ENABLE : EV_DISABLE), 0, 0, co);

    kevent(_fd, &events[0], 2, nullptr, 0, nullptr);
}

void selector::set(int fd, int event_flag, coroutine* co) {
    if (event_flag == SELECT_NONE) {
        remove(fd);
    } else {
        add(fd, event_flag, co);
    }
}

void selector::remove(int fd) {
    struct kevent events[2]; 
    EV_SET(&events[0], fd, EVFILT_READ, EV_DELETE, 0, 0, nullptr);
    EV_SET(&events[1], fd, EVFILT_WRITE, EV_DELETE, 0, 0, nullptr);

    kevent(_fd, &events[0], 2, nullptr, 0, nullptr);
}

void selector::select(int64_t ns) {
    static struct kevent events[MAX_SELECT_COUNT];

    struct timespec ts;
    ts.tv_sec = (time_t)(ns / 1'000'000'000);
    ts.tv_nsec = (long)(ns % 1'000'000'000);

    int event_cnt = kevent(_fd, nullptr, 0, events, MAX_SELECT_COUNT, &ts);
    if (event_cnt < 0) {
        if (errno == EINTR)
            return;

        perror("kevent");
        return;
    }

    for (int i = 0; i < event_cnt; ++i) {
        unsigned filt = events[i].filter;
        int event_type = 0;
        if (filt == EVFILT_READ) {
            event_type = SELECT_READ;
        } else if (filt == EVFILT_WRITE) {
            event_type = SELECT_WRITE;
        }

        auto co = (coroutine*)events[i].udata;
        co->set_value(event_type);
        co->resume();
    }
}

#endif // __APPLE__