#pragma once

namespace asy {

class coroutine;
class timer;

struct context {
    coroutine* co = nullptr;
    timer* ti = nullptr;
};

context* init_context();
context* get_context();

} // asy
