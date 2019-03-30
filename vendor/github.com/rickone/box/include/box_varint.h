#pragma once

#include <cstddef> // size_t
#include <string>

namespace box {

inline bool is_varint_header(uint8_t header) {
    return (header & 0xC0) != 0xC0;
}

inline uint32_t zigzag_encode32(int32_t value) {
    return (uint32_t)((value << 1) ^ (value >> 31));
}

inline int32_t zigzag_decode32(uint32_t value) {
    return (int32_t)((value >> 1) ^ (-(int32_t)(value & 1)));
}

inline uint64_t zigzag_encode64(int64_t value) {
    return (uint64_t)((value << 1) ^ (value >> 63));
}

inline int64_t zigzag_decode64(uint64_t value) {
    return (int64_t)((value >> 1) ^ (-(int64_t)(value & 1)));
}

void store_varint(std::string& str, uint64_t value);
uint64_t load_varint(const std::string& str, size_t pos, size_t* used_len);

} // box
