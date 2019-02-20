#include "asyn_timer.h"

using namespace asyn;

void timer::add_trigger(int64_t ns, trigger* trigger) {
    auto tp = std::chrono::steady_clock::now() + std::chrono::nanoseconds(ns);
    _skiplist.create(tp, trigger);
}

int64_t timer::tick() {
    auto tp = std::chrono::steady_clock::now();

    unsigned int n = _skiplist.upper_rank(tp);
    if (n == 0) {
        return MIN_TRIGGER_NANO_SECONDS;
    }

    auto node = _skiplist.remove(0, n);
    while (node) {
        auto next = node->forward(0);
        auto trigger = node->get_value();
        trigger->on_timer();

        node->purge();
        node = next;
    }

    auto first_node = _skiplist.first_node();
    if (!first_node) {
        return MIN_TRIGGER_NANO_SECONDS;
    }

    auto next_tick_tp = first_node->get_key();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(next_tick_tp - tp).count();
}