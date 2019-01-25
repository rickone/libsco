#include "asyn.h"

using namespace asyn;

guard g_guard_inst;

guard::guard() {
    master::inst()->enter();
}

guard::~guard() {
    master::inst()->quit();
}
