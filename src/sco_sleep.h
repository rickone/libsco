#pragma once

#include <chrono>

namespace sco {

void sleep_until(const std::chrono::steady_clock::time_point& tp);

template <class Rep, class Period>
inline void sleep_for(const std::chrono::duration<Rep, Period>& dtn) {
    auto tp = std::chrono::steady_clock::now() + dtn;
    sleep_until(tp);
}

} // sco
