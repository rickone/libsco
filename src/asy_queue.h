#pragma once

#include <atomic>

namespace asy {

class queue {
public:
    struct node_t {
        node_t* next;
        size_t len;
        char data[0];
    };

    queue();
    virtual ~queue();

    void push(const char* data, size_t len);
    node_t* pop();

private:
    std::atomic<node_t*> _head;
    std::atomic<node_t*> _tail;
};

} // asy
