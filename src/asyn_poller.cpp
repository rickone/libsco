#include "asyn_poller.h"
#include <unistd.h>
#include <cerrno>
#include "asyn_worker.h"
#include "asyn_panic.h"

using namespace asyn;

#ifdef __linux__
#include <sys/epoll.h>

poller::~poller() {
    if (_fd >= 0) {
        close(_fd);
    }
}

void poller::init() {
    _fd = epoll_create(1024);
}

void poller::add(int fd, int event_flag, coroutine* co) {
    struct epoll_event event;
    event.events = EPOLLET;
    event.data.ptr = co;

    if (event_flag & EVENT_READ) {
        event.events |= EPOLLIN;
    }

    if (event_flag & EVENT_WRITE) {
        event.events |= EPOLLOUT;
    }

    epoll_ctl(_fd, EPOLL_CTL_ADD, fd, &event);
}

void poller::set(int fd, int event_flag, coroutine* co) {
    struct epoll_event event;
    event.events = EPOLLET;
    event.data.ptr = co;

    if (event_flag & EVENT_READ) {
        event.events |= EPOLLIN;
    }

    if (event_flag & EVENT_WRITE) {
        event.events |= EPOLLOUT;
    }

    epoll_ctl(_fd, EPOLL_CTL_MOD, fd, &event);
}

void poller::remove(int fd) {
    epoll_ctl(_fd, EPOLL_CTL_DEL, fd, nullptr);
}

void poller::poll(int64_t ns) {
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
            event_type |= EVENT_READ;
        }
        if (es & EPOLLOUT) {
            event_type |= EVENT_WRITE;
        }
        auto co = (coroutine*)events[i].data.ptr;
        co->set_value(event_type);
        co->resume();
    }
}
#endif // __linux__

#ifdef __APPLE__
#include <sys/event.h>

poller::~poller() {
    if (_fd >= 0) {
        close(_fd);
    }
}

void poller::init() {
    _fd = kqueue();
}

void poller::add(int fd, int event_flag, coroutine* co) {
    struct kevent events[2];
    EV_SET(&events[0], fd, EVFILT_READ,  EV_ADD | ((event_flag & EVENT_READ) ? EV_ENABLE : EV_DISABLE), 0, 0, co);
    EV_SET(&events[1], fd, EVFILT_WRITE, EV_ADD | ((event_flag & EVENT_WRITE) ? EV_ENABLE : EV_DISABLE), 0, 0, co);

    kevent(_fd, &events[0], 2, nullptr, 0, nullptr);
}

void poller::set(int fd, int event_flag, coroutine* co) {
    if (event_flag == EVENT_NONE) {
        remove(fd);
    } else {
        add(fd, event_flag, co);
    }
}

void poller::remove(int fd) {
    struct kevent events[2]; 
    EV_SET(&events[0], fd, EVFILT_READ, EV_DELETE, 0, 0, nullptr);
    EV_SET(&events[1], fd, EVFILT_WRITE, EV_DELETE, 0, 0, nullptr);

    kevent(_fd, &events[0], 2, nullptr, 0, nullptr);
}

void poller::poll(int64_t ns) {
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
            event_type = EVENT_READ;
        } else if (filt == EVFILT_WRITE) {
            event_type = EVENT_WRITE;
        }

        auto co = (coroutine*)events[i].udata;
        co->set_value(event_type);
        co->resume();
    }
}

#endif // __APPLE__

void poller::wait(int fd, int event_flag) {
    auto cur_worker = worker::current();
    if (!cur_worker) {
        panic("!cur_worker");
    }

    auto co = cur_worker->co_self();
    if (!co) {
        panic("!co");
    }

    add(fd, event_flag, co);
    co->yield();
    remove(fd);
}
