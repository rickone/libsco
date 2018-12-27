#include "asy_queue.h"
#include <cstdlib>
#include <cstring>

using namespace asy;

static queue::node_t* create_node(const char* data, size_t len) {
    auto node = (queue::node_t*)malloc(sizeof(queue::node_t) + len);
    node->next = nullptr;
    node->len = len;
    if (data) {
        memcpy(node->data, data, len);
    }
    return node;
}

queue::queue() {
    auto dummy = create_node(nullptr, 0);
    _head = dummy;
    _tail = dummy;
}

queue::~queue() {
    // lock?
    for (auto node = _head.load(); node != nullptr; ) {
        auto next = node->next;
        free(node);
        node = next;
    }
    _head = nullptr;
    _tail = nullptr;
}

void queue::push(const char* data, size_t len) {
    node_t* new_node = create_node(data, len);
    node_t* node = _tail.load(std::memory_order_consume);

    while (!_tail.compare_exchange_weak(node, new_node, std::memory_order_release, std::memory_order_consume));

    node->next = new_node;
}

queue::node_t* queue::pop() {
    node_t* node = _head.load(std::memory_order_consume);
    node_t* next = nullptr;
    do {
        next = node->next;
        if (next == nullptr)
            return nullptr;

    } while (!_head.compare_exchange_weak(node, next, std::memory_order_release, std::memory_order_consume));

    free(node);
    return next;
}
