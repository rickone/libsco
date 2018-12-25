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
    co->resume();
    co->resume();

    co_yield();
    puts("foo.end");
}

void thread_func() {
    coroutine::setup();

    auto co = co_create(foo);
    co->resume();
    co->resume();
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

