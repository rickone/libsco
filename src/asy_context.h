#pragma once

namespace asy {

class coroutine;
class timer;
class selector;

struct context {
    coroutine* self = nullptr;
    timer* timer = nullptr;
    selector* selector = nullptr;
};

context* init_context();
context* get_context();

} // asy
