#pragma once

#include "asyn_chan.h"

namespace asyn {

template<typename T>
class future {
public:
    future() = default;
    ~future() = default;

    future(const future& other) : _chan(other._chan) {
    }

    future(future&& other) : _chan(other._chan) {
    }

    void done(const T& val) {
        _chan.send(val);
    }

    T wait() {
        return _chan.recv<T>();
    }

private:
    chan _chan;
};

template<>
class future<void> {
public:
    future() : _chan() {};
    ~future() = default;

    future(const future& other) : _chan(other._chan) {
    }

    future(future&& other) : _chan(other._chan) {
    }

    void done() {
        _chan.send(nullptr);
    }

    void wait() {
        _chan.recv<std::nullptr_t>();
    }

private:
    chan _chan;
};

} // asyn
