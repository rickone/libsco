#pragma once

#include <atomic>
#include <string>

namespace asyn {

template<typename T>
class queue {
public:
    struct node_t {
        node_t() = default;
        node_t(const T& t) : value(t) {}
        ~node_t() = default;

        node_t* next = nullptr;
        T value;
    };

    queue() {
        auto dummy = new node_t();
        _head = dummy;
        _tail = dummy;
    }

    virtual ~queue() {
        // lock?
        for (auto node = _head.load(); node != nullptr; ) {
            auto next = node->next;
            delete node;
            node = next;
        }
        _head = nullptr;
        _tail = nullptr;
    }

    void push(const T& t) {
        node_t* new_node = new node_t(t);
        node_t* node = _tail.load(std::memory_order_consume);

        while (!_tail.compare_exchange_weak(node, new_node, std::memory_order_release, std::memory_order_consume));

        node->next = new_node;
    }

    bool pop(T& value) {
        node_t* node = _head.load(std::memory_order_consume);
        node_t* next = nullptr;
        do {
            next = node->next;
            if (next == nullptr)
                return false;

        } while (!_head.compare_exchange_weak(node, next, std::memory_order_release, std::memory_order_consume));

        value = next->value;
        delete node;
        return true;
    }

private:
    std::atomic<node_t*> _head;
    std::atomic<node_t*> _tail;
};

} // asyn
