#pragma once

#include "box.h"

namespace asyn {

class coroutine;

class iterator {
public:
    iterator() = default;
    ~iterator() = default;
    explicit iterator(coroutine* co);
    iterator(const iterator& other);
    iterator(iterator&& other);
    iterator& operator=(const iterator& other);
    iterator& operator=(iterator&& other);

    box::object& operator*();
    iterator& operator++();
    iterator operator++(int);
    bool operator==(const iterator& other);
    bool operator!=(const iterator& other);

private:
    coroutine* _coroutine;
    box::object _obj;
};

} // asyn
