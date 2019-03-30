#pragma once

#include <cstddef> // size_t
#include <cstring>
#include <string>
#include <list>
#include <vector>
#include <map>
#include "box_varint.h"

/* Box Code
*
* Varient Int:
* 0xxx,xxxx(0x00 - 0x7F) -- 1 Byte
* 10xx,xxxx(0x80 - 0xBF) -> [1xxx,xxxx] -> [0xxx,xxxx] -- n Bytes (littile-endian)
*
* (0xC0+)
* 0 - nil/nullptr +[0]
* 1 - false +[0]
* 2 - true +[0]
* 3 - number/float +[4]
* 4 - number/double +[8]
* 5 - string/const char* +(len:varint) +[len]
* 6 - list/vector +(n:varint) +(value) x n
* 7 - dict/map +(n:varint) +(key,value) x n
*/

namespace box {

enum {
    HEADER_NULL = 0xC0,
    HEADER_FALSE,
    HEADER_TRUE,
    HEADER_FLOAT,
    HEADER_DOUBLE,
    HEADER_STRING,
    HEADER_LIST,
    HEADER_DICT,
};

// basic string
inline void store_string(std::string& str, const char* data, size_t len) {
    str.push_back((char)HEADER_STRING);
    store_varint(str, len);
    str.append(data, len);
    str.push_back(0);
}

inline const char* load_string(const std::string& str, size_t pos, size_t* str_len, size_t* used_len) {
    uint8_t header = (uint8_t)str.at(pos);
    if (header != HEADER_STRING) {
        throw std::range_error("box string header illegal");
    }

    size_t str_len_len = 0;
    *str_len = load_varint(str, pos + 1, &str_len_len);

    const char* result = str.c_str() + pos + 1 + str_len_len;
    *used_len = 1 + str_len_len + *str_len + 1;

    return result;
}

template<typename T>
struct special {
    static void store(std::string& str, T t) {
        static_assert(std::is_pointer<T>::value, "should be a pointer to struct");
        store_string(str, (const char*)t, sizeof(*t));
    }

    static T load(const std::string& str, size_t pos, size_t* used_len) {
        static_assert(std::is_pointer<T>::value, "should be a pointer to struct");

        size_t len = 0;
        return (T)load_string(str, pos, &len, used_len);
    }
};

// nullptr
template<>
struct special<std::nullptr_t> {
    static void store(std::string& str, std::nullptr_t ptr) {
        str.push_back((char)HEADER_NULL);
    }

    static std::nullptr_t load(const std::string& str, size_t pos, size_t* used_len) {
        uint8_t header = (uint8_t)str.at(pos);
        if (header != HEADER_NULL) {
            throw std::range_error("box nil header illegal");
        }

        *used_len = 1;
        return nullptr;
    }
};

// int types
#define box_int_def(c_type, width) \
    template<> \
    struct special<c_type> { \
        static void store(std::string& str, c_type value) { \
            store_varint(str, zigzag_encode##width(value)); \
        } \
        static c_type load(const std::string& str, size_t pos, size_t* used_len) { \
            return (c_type)zigzag_decode##width((uint32_t)load_varint(str, pos, used_len)); \
        } \
    }; \
    template<> \
    struct special<unsigned c_type> { \
        static void store(std::string& str, unsigned c_type value) { \
            store_varint(str, value); \
        } \
        static unsigned c_type load(const std::string& str, size_t pos, size_t* used_len) { \
            return (unsigned c_type)load_varint(str, pos, used_len); \
        } \
    }

box_int_def(char, 32);
box_int_def(short, 32);
box_int_def(int, 32);
box_int_def(long, 64);
box_int_def(long long, 64);

// bool
template<>
struct special<bool> {
    static void store(std::string& str, bool value) {
        str.push_back(value ? HEADER_TRUE : HEADER_FALSE);
    }

    static bool load(const std::string& str, size_t pos, size_t* used_len) {
        uint8_t header = (uint8_t)str.at(pos);
        *used_len = 1;

        if (header == HEADER_FALSE) {
            return false;
        }

        if (header == HEADER_TRUE) {
            return true;
        }

        throw std::range_error("box bool header illegal");
    }
};

// float
template<>
struct special<float> {
    static void store(std::string& str, float value) {
        str.push_back((char)HEADER_FLOAT);
        str.append((const char*)&value, sizeof(value));
    }

    static float load(const std::string& str, size_t pos, size_t* used_len) {
        uint8_t header = (uint8_t)str.at(pos);
        if (header != HEADER_FLOAT) {
            throw std::range_error("box float header illegal");
        }

        float value = 0;
        str.copy((char*)&value, sizeof(value), pos + 1);
        *used_len = 1 + sizeof(float);
        return value;
    }
};

// double
template<>
struct special<double> {
    static void store(std::string& str, double value) {
        str.push_back((char)HEADER_DOUBLE);
        str.append((const char*)&value, sizeof(value));
    }

    static double load(const std::string& str, size_t pos, size_t* used_len) {
        uint8_t header = (uint8_t)str.at(pos);
        if (header != HEADER_DOUBLE) {
            throw std::range_error("box double header illegal");
        }

        double value = 0;
        str.copy((char*)&value, sizeof(value), pos + 1);
        *used_len = 1 + sizeof(double);
        return value;
    }
};

// const char*
template<>
struct special<const char*> {
    static void store(std::string& str, const char* value) {
        size_t len = strlen(value);
        store_string(str, value, len);
    }

    static const char* load(const std::string& str, size_t pos, size_t* used_len) {
        size_t len = 0;
        return load_string(str, pos, &len, used_len);
    }
};

// std::string
template<>
struct special<std::string> {
    static void store(std::string& str, const std::string& value) {
        store_string(str, value.data(), value.size());
    }

    static std::string load(const std::string& str, size_t pos, size_t* used_len) {
        size_t len = 0;
        const char* data = load_string(str, pos, &len, used_len);
        return std::string(data, len);
    }
};

// std::list
template<typename T>
struct special< std::list<T> > {
    static void store(std::string& str, const std::list<T>& lst) {
        str.push_back(HEADER_LIST);
        store_varint(str, lst.size());
        for (const T& t : lst) {
            special<T>::store(str, t);
        }
    }

    static std::list<T> load(const std::string& str, size_t pos, size_t* used_len) {
        uint8_t header = (uint8_t)str.at(pos);
        if (header != HEADER_LIST) {
            throw std::range_error("box list header illegal");
        }

        size_t lst_len_len = 0;
        size_t lst_len = load_varint(str, pos + 1, &lst_len_len);

        std::list<T> result;
        size_t offset = pos + 1 + lst_len_len;
        for (size_t i = 0; i < lst_len; ++i) {
            size_t len = 0;
            T item = special<T>::load(str, offset, &len);
            offset += len;
            result.push_back(item);
        }
        *used_len = offset - pos;
        return result;
    }
};

// std::vector
template<typename T>
struct special< std::vector<T> > {
    static void store(std::string& str, const std::vector<T>& lst) {
        str.push_back((char)HEADER_LIST);
        store_varint(str, lst.size());
        for (const T& t : lst) {
            special<T>::store(str, t);
        }
    }

    static std::vector<T> load(const std::string& str, size_t pos, size_t* used_len) {
        uint8_t header = (uint8_t)str.at(pos);
        if (header != HEADER_LIST) {
            throw std::range_error("box list header illegal");
        }

        size_t lst_len_len = 0;
        size_t lst_len = load_varint(str, pos + 1, &lst_len_len);

        std::vector<T> result;
        size_t offset = pos + 1 + lst_len_len;
        for (size_t i = 0; i < lst_len; ++i) {
            size_t len = 0;
            T item = special<T>::load(str, offset, &len);
            offset += len;
            result.push_back(item);
        }
        *used_len = offset - pos;
        return result;
    }
};

// std::map
template<typename K, typename V>
struct special< std::map<K, V> > {
    static void store(std::string& str, const std::map<K, V>& dict) {
        str.push_back((char)HEADER_DICT);
        store_varint(str, dict.size());
        for (const typename std::map<K, V>::value_type& pair : dict) {
            special<K>::store(str, pair.first);
            special<V>::store(str, pair.second);
        }
    }

    static std::map<K, V> load(const std::string& str, size_t pos, size_t* used_len) {
        uint8_t header = (uint8_t)str.at(pos);
        if (header != HEADER_DICT) {
            throw std::range_error("box dict header illegal");
        }

        size_t dict_len_len = 0;
        size_t dict_len = load_varint(str, pos + 1, &dict_len_len);

        std::map<K, V> result;
        size_t offset = pos + 1 + dict_len_len;
        for (size_t i = 0; i < dict_len; ++i) {
            size_t len = 0;
            K key = special<K>::load(str, offset, &len);
            offset += len;
            V value = special<V>::load(str, offset, &len);
            offset += len;
            result.insert(std::make_pair(key, value));
        }
        *used_len = offset - pos;
        return result;
    }
};

} // box
