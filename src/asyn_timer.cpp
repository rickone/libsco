#include "asyn_timer.h"
#include "asyn_context.h"

using namespace asyn;

void timer::sleep(int64_t ns, coroutine* co) {
    auto tp = std::chrono::steady_clock::now() + std::chrono::nanoseconds(ns);
    _skiplist.create(tp, co);
}

void timer::tick() {
    auto tp = std::chrono::steady_clock::now();

    unsigned int n = _skiplist.upper_rank(tp);
    if (n == 0) {
        return;
    }

    auto node = _skiplist.remove(0, n);
    while (node) {
        auto next = node->forward(0);
        auto co = node->get_value();
        co->resume();

        node->purge();
        node = next;
    }
}

void asyn::nsleep(int64_t ns) {
    auto ctx = get_context();
    ctx->timer->sleep(ns, ctx->self);
    ctx->self->yield();
}
