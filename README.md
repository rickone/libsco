# asyn
asyn 是一个基于ucontext的C++14的轻量级协程库，引入它将会把目标程序变为协程架构，协程比线程更轻量，asyn会运行指定个操作系统线程以处理用户创建的协程，不同协程之间可能运行在不同的线程，需要处理一些并行造成的问题。除此之外，asyn还会override一些阻塞的系统调用（如sleep和read/write等），让它变成阻塞协程而非线程，这样看起来同步执行的代码是在异步执行。

# Install
make

会在lib目录生成libasyn.so，在include目录放一些头文件。在使用的工程-I{include目录} -L{lib目录} -lasyn即可。

# 支持平台 Platform
目前只支持MacOS和Linux

# 环境变量 env
ASYN_WORKER_NUM

操作系统线程数，它表示真正的并行能力，默认值是CPU核心数。

ASYN_BIND_CPU_CORE

=1将把从0到CPU_NUM的线程绑定到对应CPU核。
默认不绑定

# Hello World
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

# Wait 最基本的同步
``` C++
#include "asyn.h"
#include <cstdio>

using namespace std::chrono_literals;

int foo() {
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        printf("foo i=%d\n", i);
        asyn::sleep_for(10ms);
        sum += i;
    }
    return sum;
}

int main() {
    auto ch = asyn::start(foo);
    auto s = ch->wait<int>();

    printf("s=%d\n", s);
    return 0;
}
```
如果执行的协程函数有返回值定义(非void)，asyn::start()会返回一个channel对象的共享指针，可以调用channel::wait<T>()以等待协程执行的结果。多个协程会使用的对象，建议都使用std::shared_ptr封装。

# WaitGroup和Mutex
``` C++
#include "asyn.h"
#include <cstdio>
#include <mutex>

static std::mutex s_mutex;
static std::vector<int> s_result;

bool is_prime(int n) {
    if (n % 2 == 0) {
        return false;
    }

    int m = (int)sqrt(n);
    for (int i = 3; i <= m; i += 2) {
        if (n % i == 0) {
            return false;
        }
    }

    printf("%d is prime\n", n);
    return true;
}

void foo(int n) {
    if (!is_prime(n)) {
        return;
    }

    s_mutex.lock();
    s_result.push_back(n);
    s_mutex.unlock();
}

int main() {
    asyn::wait_group wg;

    for (int i = 2; i < 1000; i++) {
        wg.start(std::bind(foo, i));
    }

    wg.wait();

    puts("prime number:");
    for (int n : s_result) {
        printf("%d\n", n);
    }

    return 0;
}
```
asyn::wait_group对象可以等待多个协程全部执行结束，它不需要协程函数一定有反回值，全部从它这里start的协程全部执行结束后，wait()会返回。多
个协程需要访问的临界区需要用asyn::mutex保护，类似pthread_mutex，但不是在线程上加锁，而是在协程上加锁。

# 直接使用协程 Coroutine
