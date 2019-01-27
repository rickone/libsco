# asyn
asyn 是一个基于ucontext的C++14的协程库，引入它将会把目标程序变为协程架构，协程比线程更轻量，asyn会运行指定个操作系统线程以处理用户创建的协程，不同协程之间可能运行在不同的线程，需要处理一些并行造成的问题。除此之外，asyn还会override一些阻塞的系统调用（如sleep和read/write等），让它变成阻塞协程而非线程，这样看起来同步执行的代码是在异步执行。

# Install
make

会在lib目录生成libasyn.so，在include目录放一些头文件。在使用的工程-I{include目录} -L{lib目录} -lasyn即可。

# 支持平台
目前只支持MacOS和Linux

# HelloWorld
``` C++
#include "asyn.h"
#include <cstdio>

using namespace std::chrono_literals;

void foo(int i) {
    printf("[%d] Hello World!\n", i);
}

int main() {
    for (int i = 0; i < 20; i++) {
        asyn::start(std::bind(foo, i));
    }

    asyn::sleep_for(1s);
    return 0;
}

```
引入asyn库后，main函数过程将变身为主协程，主协程退出后进程会退出。main的返回值将会被忽略，使用asyn::quit(code)在需要指定退出码的时候，否则main函数退出进程返回0。

asyn::start()启动一个异步协程并执行，当前协程继续执行。为了不让main退出，使用asyn::sleep_for()等待1秒，你还可以使用std::this_thread::sleep_for, sleep, usleep, nanosleep，他们都被替换成asyn::sleep_for，效果是一样的。见test/test_sleep.cpp
