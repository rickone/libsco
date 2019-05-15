#include "sco.h"

using namespace sco;

void sco::sleep_until(const std::chrono::steady_clock::time_point& tp) {
    auto self = routine::self();
    runtime_assert(self, "");

    int64_t timeout_usec = std::chrono::duration_cast<std::chrono::microseconds>(tp - std::chrono::steady_clock::now()).count();
    if (timeout_usec <= 0) {
        return;
    }

    self->wait_event(-1, 0, timeout_usec);
}
