#include "asy_uthread.h"

using namespace asy;

uthread::uthread(func_t* func, void* arg) : _func(func), _arg(arg) {
}

uthread::~uthread() {
    if (_co) {
        delete _co;
    }
}

bool uthread::activate() {
    switch (_status) {
        case UTHREAD_READY: {
            _co = new asy::coroutine();
            std::function<void(void)> func = [this](){
                _func(_arg);
            };
            _co->bind(func);
            resume();
            return true;
        }

        case UTHREAD_PENDING: {
            auto node = _queue.pop();
            if (node) {
                if (node->len > 0) {
                    xbin::object obj(node->data, node->len);
                    _co->move_value(std::move(obj));
                } else {
                    xbin::object obj;
                    obj.store(nullptr);
                    _co->move_value(std::move(obj));
                }
                
                resume();
            }
            return true;
        }
    }

    return false;
}

void uthread::resume() {
    try {
        _co->resume();
        int status = _co->status();
        if (status == COROUTINE_SUSPEND) {
            _status = UTHREAD_PENDING;
        } else if (status == COROUTINE_DEAD) {
            _status = UTHREAD_FINISH;
        }
    } catch (const std::runtime_error &err) {
        fprintf(stderr, "uthread runtime_error: %s\n", err.what());
        _status = UTHREAD_FINISH;
    }
}
