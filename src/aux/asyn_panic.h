#pragma once

#include <string>
#include <stdexcept>

namespace asyn {

std::string backtrace(const char *fmt, ...);
std::string backtrace_system(const char *name);

#define panic(fmt, ...) throw std::runtime_error(backtrace(fmt,## __VA_ARGS__))
#define panic_system(name) throw std::runtime_error(backtrace_system(name))

} // asyn
