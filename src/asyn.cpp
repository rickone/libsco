#include "asyn.h"

using namespace asyn;

guard g_guard_inst;

guard::guard() {
    master::inst()->enter();
}

guard::~guard() {
    master::inst()->quit();
}

void asyn::sleep_until(const std::chrono::steady_clock::time_point& tp) {
    int64_t timeout_usec = std::chrono::duration_cast<std::chrono::microseconds>(tp - std::chrono::steady_clock::now()).count();
    if (timeout_usec <= 0) {
        return;
    }

    wait_event(-1, 0, timeout_usec);
}
