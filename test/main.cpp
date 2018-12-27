#include "cstdio"
#include <thread>

using namespace std::chrono_literals;

int main() {
    for (int i = 0; i < 10; ++i) {
        printf("i=%d\n", i);
        std::this_thread::sleep_for(60ms);
    }
    return 0;
}

