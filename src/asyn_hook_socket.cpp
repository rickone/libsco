#include "asyn_dlfunc.h"
#include <unistd.h> // close
#include <fcntl.h>
#include <sys/socket.h> // socklen_t
#include <sys/un.h> // AF_UNIX
#include <netdb.h> // getnameinfo
#include <netinet/tcp.h> // TCP_NODELAY
#include "asyn_master.h"

using namespace asyn;

static bool add_nonblock(int fd) {
    int option = fcntl(fd, F_GETFL);
    if (option == -1) {
        return false;
    }
    
    int ret = fcntl(fd, F_SETFL, option | O_NONBLOCK);
    if (ret == -1) {
        return false;
    }

    return true;
}

sys_hook(socket)
int socket(int domain, int type, int protocol) {
    if (!worker::current()) {
        return sys_org(socket)(domain, type, protocol);
    }

#ifdef __linux__
    return sys_org(socket)(domain, type | SOCK_NONBLOCK, protocol);
#else
    int fd = sys_org(socket)(domain, type, protocol);
    if (fd == -1) {
        return -1;
    }

    if (!add_nonblock(fd)) {
        close(fd);
        return -1;
    }

    return fd;
#endif
}

sys_hook(connect)
int connect(int fd, const struct sockaddr* addr, socklen_t addrlen) {
    auto w = worker::current();
    if (!w) {
        return sys_org(connect)(fd, addr, addrlen);
    }

    int ret = sys_org(connect)(fd, addr, addrlen);
    if (ret == 0) {
        return 0;
    }

    if (errno != EINPROGRESS) {
        return -1;
    }

    w->poller_inst()->wait(fd, EVENT_WRITE);
    return 0;
}

sys_hook(accept)

static int accept_nonblock(int listenfd, struct sockaddr* addr, socklen_t* addrlen) {
#ifdef __linux__
    return accept4(listenfd, addr, addrlen, O_NONBLOCK);
#else
    int fd = sys_org(accept)(listenfd, addr, addrlen);
    if (fd == -1) {
        return -1;
    }

    if (!add_nonblock(fd)) {
        close(fd);
        return -1;
    }

    return fd;
#endif
}

int accept(int listenfd, struct sockaddr* addr, socklen_t* addrlen) {
    auto w = worker::current();
    if (!w) {
        return sys_org(accept)(listenfd, addr, addrlen);
    }

    int fd = accept_nonblock(listenfd, addr, addrlen);
    if (fd != -1) {
        return fd;
    }

    if (errno != EAGAIN) {
        return -1;
    }

    w->poller_inst()->wait(fd, EVENT_READ);
    return accept_nonblock(listenfd, addr, addrlen);
}

sys_hook(recv)
ssize_t recv(int fd, void* data, size_t len, int flags) {
    ssize_t ret = sys_org(recv)(fd, data, len, flags);
    if (ret >= 0) {
        return ret;
    }

    if (errno != EAGAIN) {
        return -1;
    }

    auto w = worker::current();
    if (!w) {
        return -1;
    }

    w->poller_inst()->wait(fd, EVENT_READ);
    return sys_org(recv)(fd, data, len, flags);
}

sys_hook(recvfrom)
ssize_t recvfrom(int fd, void* data, size_t len, int flags, struct sockaddr* addr, socklen_t* addrlen) {
    ssize_t ret = sys_org(recvfrom)(fd, data, len, flags, addr, addrlen);
    if (ret >= 0) {
        return ret;
    }

    if (errno != EAGAIN) {
        return -1;
    }

    auto w = worker::current();
    if (!w) {
        return -1;
    }

    w->poller_inst()->wait(fd, EVENT_READ);
    return sys_org(recvfrom)(fd, data, len, flags, addr, addrlen);
}

sys_hook(recvmsg)
ssize_t recvmsg(int fd, struct msghdr* msg, int flags) {
    ssize_t ret = sys_org(recvmsg)(fd, msg, flags);
    if (ret >= 0) {
        return ret;
    }

    if (errno != EAGAIN) {
        return -1;
    }

    auto w = worker::current();
    if (!w) {
        return -1;
    }

    w->poller_inst()->wait(fd, EVENT_READ);
    return sys_org(recvmsg)(fd, msg, flags);
}

sys_hook(send)
ssize_t send(int fd, const void* data, size_t len, int flags) {
    ssize_t ret = sys_org(send)(fd, data, len, flags);
    if (ret >= 0) {
        return ret;
    }

    if (errno != EAGAIN) {
        return -1;
    }

    auto w = worker::current();
    if (!w) {
        return -1;
    }

    w->poller_inst()->wait(fd, EVENT_WRITE);
    return sys_org(send)(fd, data, len, flags);
}

sys_hook(sendto)
ssize_t sendto(int fd, const void* data, size_t len, int flags, const struct sockaddr* addr, socklen_t addrlen) {
    ssize_t ret = sys_org(sendto)(fd, data, len, flags, addr, addrlen);
    if (ret >= 0) {
        return ret;
    }

    if (errno != EAGAIN) {
        return -1;
    }

    auto w = worker::current();
    if (!w) {
        return -1;
    }

    w->poller_inst()->wait(fd, EVENT_WRITE);
    return sys_org(sendto)(fd, data, len, flags, addr, addrlen);
}

sys_hook(sendmsg)
ssize_t sendmsg(int fd, const struct msghdr* msg, int flags) {
    ssize_t ret = sys_org(sendmsg)(fd, msg, flags);
    if (ret >= 0) {
        return ret;
    }

    if (errno != EAGAIN) {
        return -1;
    }

    auto w = worker::current();
    if (!w) {
        return -1;
    }

    w->poller_inst()->wait(fd, EVENT_WRITE);
    return sys_org(sendmsg)(fd, msg, flags);
}

sys_hook(pread)
ssize_t pread(int fd, void* data, size_t len, off_t offset) {
    ssize_t ret = sys_org(pread)(fd, data, len, offset);
    if (ret >= 0) {
        return ret;
    }

    if (errno != EAGAIN) {
        return -1;
    }

    auto w = worker::current();
    if (!w) {
        return -1;
    }

    w->poller_inst()->wait(fd, EVENT_READ);
    return sys_org(pread)(fd, data, len, offset);
}

sys_hook(read)
ssize_t read(int fd, void* data, size_t len) {
    ssize_t ret = sys_org(read)(fd, data, len);
    if (ret >= 0) {
        return ret;
    }

    if (errno != EAGAIN) {
        return -1;
    }

    auto w = worker::current();
    if (!w) {
        return -1;
    }

    w->poller_inst()->wait(fd, EVENT_READ);
    return sys_org(read)(fd, data, len);
}

sys_hook(readv)
ssize_t readv(int fd, const struct iovec* iov, int iovcnt) {
    ssize_t ret = sys_org(readv)(fd, iov, iovcnt);
    if (ret >= 0) {
        return ret;
    }

    if (errno != EAGAIN) {
        return -1;
    }

    auto w = worker::current();
    if (!w) {
        return -1;
    }

    w->poller_inst()->wait(fd, EVENT_READ);
    return sys_org(readv)(fd, iov, iovcnt);
}

sys_hook(pwrite)
ssize_t pwrite(int fd, const void* data, size_t len, off_t offset) {
    ssize_t ret = sys_org(pwrite)(fd, data, len, offset);
    if (ret >= 0) {
        return ret;
    }

    if (errno != EAGAIN) {
        return -1;
    }

    auto w = worker::current();
    if (!w) {
        return -1;
    }

    w->poller_inst()->wait(fd, EVENT_WRITE);
    return sys_org(pwrite)(fd, data, len, offset);
}

sys_hook(write)
ssize_t write(int fd, const void* data, size_t len) {
    ssize_t ret = sys_org(write)(fd, data, len);
    if (ret >= 0) {
        return ret;
    }

    if (errno != EAGAIN) {
        return -1;
    }

    auto w = worker::current();
    if (!w) {
        return -1;
    }

    w->poller_inst()->wait(fd, EVENT_WRITE);
    return sys_org(write)(fd, data, len);
}

sys_hook(writev)
ssize_t writev(int fd, const struct iovec* iov, int iovcnt) {
    ssize_t ret = sys_org(writev)(fd, iov, iovcnt);
    if (ret >= 0) {
        return ret;
    }

    if (errno != EAGAIN) {
        return -1;
    }

    auto w = worker::current();
    if (!w) {
        return -1;
    }

    w->poller_inst()->wait(fd, EVENT_WRITE);
    return sys_org(writev)(fd, iov, iovcnt);
}
