#pragma once

#include "asy_coroutine.h"
#include "asy_queue.h"

namespace asy {

enum {
    UTHREAD_READY,
    UTHREAD_PENDING,
    UTHREAD_FINISH,
};

class uthread {
public:
    typedef void* func_t(void*);

    uthread(func_t* func, void* arg);
    virtual ~uthread();

    bool activate();
    void resume();

    template<typename... A>
    void notify(A... args) {
        xbin::object obj;
        obj.store_args(args...);
        auto& str = obj.str();
        _queue.push(str.data(), str.size());
    }

    void notify_nil() {
        _queue.push(nullptr, 0);
    }

    int status() const { return _status; }

private:
    func_t* _func;
    void* _arg;
    coroutine* _co = nullptr;
    queue _queue;
    int _status = UTHREAD_READY;
};

} // asy
