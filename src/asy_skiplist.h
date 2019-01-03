#pragma once

#include <cassert>
#include <functional> // equal_to
#include <algorithm> // min
#include <random>

namespace asy {

template<typename K, typename V, typename C, int I>
class skiplist {
public:
    class node_t {
    public:
        explicit node_t(const K& key) : _key(key), _value() {
            for (int i = 0; i < I; i++) {
                _forward_array[i] = nullptr;
                _span_array[i] = 0;
            }
        }

        virtual ~node_t() = default;

        static node_t* create(const K& key) {
            return new node_t(key);
        }

        void purge() {
            delete this;
        }

        const K& get_key() const {
            return _key;
        }

        void set_key(const K& key) {
            _key = key;
        }

        V& get_value() {
            return _value;
        }

        const V& get_value() const {
            return _value;
        }

        void set_value(const V& value) {
            _value = value;
        }

        node_t*& forward(int level) {
            assert(level >= 0 && level < I);
            return _forward_array[level];
        }

        unsigned int& span(int level) {
            assert(level >= 0 && level < I);
            return _span_array[level];
        }

    private:
        K _key;
        V _value;

        node_t* _forward_array[I];
        unsigned int _span_array[I];
    };

    explicit skiplist(int rand_max = 2) : _header_node(nullptr), _level(-1), _length(0),
        _compare_func(), _random_engine(), _random_dis(0, rand_max - 1) {
        _header_node = node_t::create(K());
    }

    virtual ~skiplist() {
        node_t* node = _header_node;
        while (node) {
            node_t* next = node->forward(0);
            node->purge();
            node = next;
        }
    }

    node_t* first_node() {
        return _header_node->forward(0);
    }

    // [key
    node_t* lower_bound(const K& key) {
        node_t* node = _header_node;
        auto equal_to = std::equal_to<K>();

        for (int i = _level; i >= 0; i--) {
            while (true) {
                node_t* next = node->forward(i);
                if (!next) {
                    break;
                }

                const K& k = next->get_key();
                if (_compare_func(key, k) || equal_to(key, k)) {
                    break;
                }

                node = next;
            }
        }

        return node->forward(0);
    }

    // (key
    node_t* upper_bound(const K& key) {
        node_t* node = _header_node;

        for (int i = _level; i >= 0; i--) {
            while (true) {
                node_t* next = node->forward(i);
                if (!next) {
                    break;
                }

                const K& k = next->get_key();
                if (_compare_func(key, k)) {
                    break;
                }

                node = next;
            }
        }

        return node->forward(0);
    }

    unsigned int lower_rank(const K& key) {
        unsigned int rank = 0;
        node_t* node = _header_node;
        auto equal_to = std::equal_to<K>();

        for (int i = _level; i >= 0; i--) {
            while (true) {
                node_t* next = node->forward(i);
                if (!next) {
                    break;
                }

                const K& k = next->get_key();
                if (_compare_func(key, k) || equal_to(key, k)) {
                    break;
                }

                rank += node->span(i);
                node = next;
            }
        }

        return rank;
    }

    unsigned int upper_rank(const K& key) {
        unsigned int rank = 0;
        node_t* node = _header_node;

        for (int i = _level; i >= 0; i--) {
            while (true) {
                node_t* next = node->forward(i);
                if (!next) {
                    break;
                }

                const K& k = next->get_key();
                if (_compare_func(key, k)) {
                    break;
                }

                rank += node->span(i);
                node = next;
            }
        }

        return rank;
    }

    node_t* operator [](unsigned int rank) {
        if (rank >= _length) {
            return nullptr;
        }

        node_t* node = _header_node;

        for (int i = _level; i >= 0 && rank > 0; i--) {
            while (true) {
                node_t* next = node->forward(i);
                if (!next) {
                    break;
                }

                unsigned int span = node->span(i);
                if (span > rank) {
                    break;
                }

                node = next;
                rank -= span;
            }
        }

        return node->forward(0);
    }

    unsigned int length() const {
        return _length;
    }

    node_t* insert(node_t* target_node, int level = -1) {
        unsigned int target_span = target_node->span(0);
        assert(target_span == 0);

        if (level < 0) {
            level = 0;
            while ((_random_dis(_random_engine) == 0) && (level < I - 1)) {
                ++level;
            }
        }

        const K& key = target_node->get_key();
        node_t* pre_nodes[I];
        unsigned int spans[I];
        unsigned int rank = 0;
        node_t* node = _header_node;

        _length++;
        
        if (level > _level) {
            for (int i = _level + 1; i <= level; i++) {
                _header_node->span(i) = _length;
            }

            _level = level;
        }

        for (int i = _level; i >= 0; i--) {
            while (true) {
                node_t* next = node->forward(i);
                if (!next) {
                    break;
                }

                const K& k = next->get_key();
                if (_compare_func(key, k)) {
                    break;
                }

                rank += node->span(i);
                node = next;
            }

            pre_nodes[i] = node;
            spans[i] = rank;
        }

        for (int i = 0; i <= level; i++) {
            node_t* pre_node = pre_nodes[i];
            node_t* next = pre_node->forward(i);
            unsigned int span = rank - spans[i];

            target_node->forward(i) = next;
            target_node->span(i) = pre_node->span(i) - span;

            pre_node->forward(i) = target_node;
            pre_node->span(i) = span + 1;
        }

        for (int i = level + 1; i <= _level; i++) {
            pre_nodes[i]->span(i)++;
        }

        return target_node;
    }

    node_t* remove(unsigned int rank, unsigned int count) {
        if (rank >= _length) {
            return nullptr;
        }

        count = std::min(_length - rank, count);
        if (count == 0) {
            return nullptr;
        }

        node_t* pre_nodes[I];
        node_t* node = _header_node;

        for (int i = _level; i >= 0; i--) {
            while (true) {
                node_t* next = node->forward(i);
                if (!next) {
                    break;
                }

                unsigned int span = node->span(i);
                if (span > rank) {
                    break;
                }

                node = next;
                rank -= span;
            }

            pre_nodes[i] = node;
        }

        node_t* result = nullptr;
        for (unsigned int j = 0; j < count; j++) {
            node_t* target_node = node->forward(0);
            assert(target_node != nullptr);

            for (int i = 0; i <= _level; i++) {
                if (pre_nodes[i]->forward(i) != target_node) {
                    break;
                }

                pre_nodes[i]->forward(i) = target_node->forward(i);
                pre_nodes[i]->span(i) += target_node->span(i);

                target_node->forward(i) = nullptr;
                target_node->span(i) = 0;
            }

            target_node->forward(0) = result;
            result = target_node;
        }

        for (int i = 0; i <= _level; i++) {
            pre_nodes[i]->span(i) -= count;
        }

        while (_level >= 0 && !_header_node->forward(_level)) {
            _header_node->span(_level) = 0;
            _level--;
        }

        _length -= count;
        return result;
    }

    void add(const K& key, int level) {
        insert(node_t::create(key), level);
    }

    node_t* create(const K& key, const V& value) {
        node_t* node = node_t::create(key);
        node->set_value(value);
        return insert(node);
    }

private:
    node_t* _header_node;
    int _level;
    unsigned int _length;
    C _compare_func;
    std::default_random_engine _random_engine;
    std::uniform_int_distribution<int> _random_dis;
};

} // asy
