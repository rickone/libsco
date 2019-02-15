#include "asyn_iterator.h"
#include "asyn_coroutine.h"

using namespace asyn;

iterator::iterator(coroutine* co) : _coroutine(co) {
    co->resume();
    _obj = co->get_value();

    if (co->status() == COROUTINE_DEAD && !_obj) {
        _coroutine = nullptr;
    }
}

iterator::iterator(const iterator& other) : _coroutine(other._coroutine), _obj(other._obj) {
}

iterator::iterator(iterator&& other) : _coroutine(other._coroutine), _obj(std::move(other._obj)) {
    other._coroutine = nullptr;
}

iterator& iterator::operator=(const iterator& other) {
    _coroutine = other._coroutine;
    _obj = other._obj;
    return *this;
}

iterator& iterator::operator=(iterator&& other) {
    _coroutine = other._coroutine;
    other._coroutine = nullptr;
    _obj = std::move(other._obj);
    return *this;
}

box::object& iterator::operator*() {
    if (_coroutine && _coroutine->status() == COROUTINE_DEAD) {
        _coroutine = nullptr;
    }
    return _obj;
}

iterator& iterator::operator++() {
    if (_coroutine && _coroutine->resume()) {
        _obj = _coroutine->get_value();
        
        if (_coroutine->status() == COROUTINE_DEAD && !_obj) {
            _coroutine = nullptr;
        }
    }
    return *this;
}

iterator iterator::operator++(int) {
    iterator tmp(*this);
    ++(*this);
    return tmp;
}

bool iterator::operator==(const iterator& other) {
    return _coroutine == other._coroutine;
}

bool iterator::operator!=(const iterator& other) {
    return _coroutine != other._coroutine;
}
