#include "coroutine.h"

void foo() {

}

int main() {
    coroutine_t::main();

    coroutine_t::create(foo, 1024 * 128);
}

