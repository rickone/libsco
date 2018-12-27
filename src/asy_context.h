#pragma once

#include "asy_coroutine.h"
#include "asy_timer.h"

namespace asy {

struct context {
    coroutine* co = nullptr;
    timer* ti = nullptr;
};

context* get_context();

} // asy
