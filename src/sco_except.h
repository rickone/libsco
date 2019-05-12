#pragma once

#include <string>
#include <stdexcept>
#include <cerrno>
#include <cstring>

namespace sco {

std::string backtrace(const char *fmt, ...);

#define runtime_throw(fmt, ...) \
    throw std::runtime_error(backtrace("[in %s at %s:%d] " fmt, __PRETTY_FUNCTION__, __FILE__, __LINE__,## __VA_ARGS__))

#define runtime_throw_std(name) \
    throw std::runtime_error(backtrace("[in %s at %s:%d] '%s' error: %d - %s", __PRETTY_FUNCTION__, __FILE__, __LINE__, name, errno, std::strerror(errno)))

#define runtime_assert(condition, fmt, ...) \
    if (!(condition)) \
        runtime_throw(fmt,## __VA_ARGS__)

#define runtime_assert_std(condition, name) \
    if (!(condition)) \
        runtime_throw_std(name)

} // sco
