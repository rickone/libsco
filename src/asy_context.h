#pragma once

namespace asy {

class coroutine;
class timer;
class poller;

struct context {
    coroutine* self = nullptr;
    timer* timer = nullptr;
    poller* poller = nullptr;
};

context* init_context();
context* get_context();

} // asy
