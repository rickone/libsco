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
    if (_fd < 0) {
        panic_system("epoll_create");
    }
}

void poller::add(int fd, int event_flag) {
    int old_event_flag = get_event_flag(fd);
    int new_event_flag = old_event_flag | event_flag;

    if (old_event_flag == new_event_flag) {
        return;
    }

    struct epoll_event event;
    event.events = EPOLLET;
    event.data.fd = fd;

    if (new_event_flag & EVENT_READ) {
        event.events |= EPOLLIN;
    }

    if (new_event_flag & EVENT_WRITE) {
        event.events |= EPOLLOUT;
    }

    if (old_event_flag == 0) {
        epoll_ctl(_fd, EPOLL_CTL_ADD, fd, &event);        
    } else {
        epoll_ctl(_fd, EPOLL_CTL_MOD, fd, &event);
    }

    _fd_event_flags[fd] = new_event_flag;
}

void poller::remove(int fd, int event_flag) {
    int old_event_flag = get_event_flag(fd);
    int new_event_flag = old_event_flag &~ event_flag;

    if (old_event_flag == new_event_flag) {
        return;
    }

    if (new_event_flag == 0) {
        epoll_ctl(_fd, EPOLL_CTL_DEL, fd, nullptr);
        _fd_event_flags.erase(fd);
        return;
    }

    struct epoll_event event;
    event.events = EPOLLET;
    event.data.fd = fd;

    if (new_event_flag & EVENT_READ) {
        event.events |= EPOLLIN;
    }

    if (new_event_flag & EVENT_WRITE) {
        event.events |= EPOLLOUT;
    }

    epoll_ctl(_fd, EPOLL_CTL_MOD, fd, &event);
    _fd_event_flags[fd] = new_event_flag;
}

void poller::poll(int64_t ns) {
    static struct epoll_event events[MAX_SELECT_COUNT];
    
    int event_cnt = epoll_wait(_fd, events, MAX_SELECT_COUNT, (int)(ns / 1'000'000));
    if (event_cnt < 0) {
        if (errno == EINTR)
            return;

        panic_system("epoll_wait");
        return;
    }

    for (int i = 0; i < event_cnt; ++i) {
        unsigned es = events[i].events;
        int fd = events[i].data.fd;
        if (es & EPOLLIN) {
            resume_read(fd);
        }

        if (es & EPOLLOUT) {
            resume_write(fd);
        }
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
    if (_fd < 0) {
        panic_system("kqueue");
    }
}

void poller::add(int fd, int event_flag) {
    int old_event_flag = get_event_flag(fd);
    int new_event_flag = old_event_flag | event_flag;

    if (old_event_flag == new_event_flag) {
        return;
    }

    struct kevent events[2];
    void* udata = (void*)(intptr_t)fd;
    EV_SET(&events[0], fd, EVFILT_READ,  EV_ADD | ((new_event_flag & EVENT_READ) ? EV_ENABLE : EV_DISABLE), 0, 0, udata);
    EV_SET(&events[1], fd, EVFILT_WRITE, EV_ADD | ((new_event_flag & EVENT_WRITE) ? EV_ENABLE : EV_DISABLE), 0, 0, udata);

    kevent(_fd, &events[0], 2, nullptr, 0, nullptr);
}

void poller::remove(int fd, int event_flag) {
    int old_event_flag = get_event_flag(fd);
    int new_event_flag = old_event_flag | event_flag;

    if (old_event_flag == new_event_flag) {
        return;
    }

    if (new_event_flag == 0) {
        struct kevent events[2];
        void* udata = (void*)(intptr_t)-1;
        EV_SET(&events[0], fd, EVFILT_READ, EV_DELETE, 0, 0, udata);
        EV_SET(&events[1], fd, EVFILT_WRITE, EV_DELETE, 0, 0, udata);

        kevent(_fd, &events[0], 2, nullptr, 0, nullptr);
        _fd_event_flags.erase(fd);
        return;
    }

    struct kevent events[2];
    void* udata = (void*)(intptr_t)fd;
    EV_SET(&events[0], fd, EVFILT_READ,  EV_ADD | ((new_event_flag & EVENT_READ) ? EV_ENABLE : EV_DISABLE), 0, 0, udata);
    EV_SET(&events[1], fd, EVFILT_WRITE, EV_ADD | ((new_event_flag & EVENT_WRITE) ? EV_ENABLE : EV_DISABLE), 0, 0, udata);

    kevent(_fd, &events[0], 2, nullptr, 0, nullptr);
    _fd_event_flags[fd] = new_event_flag;
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

        panic_system("kevent");
        return;
    }

    for (int i = 0; i < event_cnt; ++i) {
        unsigned filt = events[i].filter;
        int fd = (int)(intptr_t)events[i].udata;

        if (filt == EVFILT_READ) {
            resume_read(fd);
        } else if (filt == EVFILT_WRITE) {
            resume_write(fd);
        }
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

    add(fd, event_flag);

    if (event_flag & EVENT_READ) {
        _read_wait_cos.emplace(fd, co->shared_from_this());
    } else if (event_flag & EVENT_WRITE) {
        _write_wait_cos.emplace(fd, co->shared_from_this());
    }

#ifdef ASYN_DEBUG
    printf("[ASYN] coroutine(%d) poller::wait, fd=%d, event=%d\n", co->id(), fd, event_flag);
#endif
    co->yield();
}

void poller::resume_read(int fd) {
    auto it = _read_wait_cos.find(fd);
    if (it == _read_wait_cos.end()) {
        return;
    }

    auto co = it->second;
    _read_wait_cos.erase(it);

    if (co) {
#ifdef ASYN_DEBUG
        printf("[ASYN] coroutine(%d) poller::resume_read, fd=%d\n", co->id(), fd);
#endif
        co->resume();
    }
}

void poller::resume_write(int fd) {
    auto it = _write_wait_cos.find(fd);
    if (it == _write_wait_cos.end()) {
        return;
    }

    auto co = it->second;
    _write_wait_cos.erase(it);

    if (co) {
#ifdef ASYN_DEBUG
        printf("[ASYN] coroutine(%d) poller::resume_write, fd=%d\n", co->id(), fd);
#endif
        co->resume();
    }
}

int poller::get_event_flag(int fd) {
    auto it = _fd_event_flags.find(fd);
    if (it == _fd_event_flags.end()) {
        return 0;
    }
    return it->second;
}
