#include "coroutine.h"
#include <thread>

void hello() {
    puts("Hello!");

    co_yield();
    puts("World!");
}

void foo() {
    puts("foo");

    auto co = co_create(hello);
    co_resume(co);
    co_resume(co);

    for (int i = 0; i < 10; ++i) {
        co_yield(i);
    }
    puts("foo.end");
}

void thread_func() {
    coroutine::setup();

    auto co = co_create(foo);
    co_resume(co);

    while (co->status() != COROUTINE_DEAD) {
        auto arg = co_resume(co);
        printf("resume: %d\n", arg.load<int>());
    }
}

int main() {
    puts("main");

    std::thread t1(thread_func);
    std::thread t2(thread_func);

    thread_func();

    t1.join();
    t2.join();

    puts("main.end");
    return 0;
}

