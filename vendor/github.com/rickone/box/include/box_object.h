#pragma once

#include <unordered_set>
#include <memory>
#include <cstdio>
#include "box_special.h"

namespace box {

class object;

struct interface {
    virtual void store(object* proto) = 0;
    virtual void load(object* proto) = 0;
};

class object {
public:
    object() = default;
    virtual ~object() = default;
    object(std::nullptr_t) {}

    object(const char* data, size_t len) : _str(data, len) {}
    explicit object(const std::string& str) : _str(str) {}
    object(const object& other) : _str(other._str) {}
    object(object&& other) : _str(std::move(other._str)), _pos(other._pos), _load_objs(std::move(other._load_objs)) {}

    object& operator=(const object& other){
        _str = other._str;
        _pos = 0;
        _load_objs.clear();
        return *this;
    }

    object& operator=(object&& other){
        _str = std::move(other._str);
        _pos = other._pos;
        _load_objs = std::move(other._load_objs);
        return *this;
    }

    void clear() {
        _str.clear();
        _pos = 0;
        _load_objs.clear();
    }

    std::string dump() {
        std::string result = "result=(";
        for (char c : _str)
        {
            static char temp[32];
            sprintf(temp, "%02x,", (unsigned char)c);
            result += temp;
        }
        result += ")";
        return result;
    }

    template<typename T>
    void store(T t) {
        store_impl(
            t,
            typename std::is_base_of<interface, typename std::decay<typename std::remove_pointer<T>::type>::type>::type()
        );
    }

    template<typename T, typename C>
    void store_impl(T t, C&&) {
        special<T>::store(_str, t);
    }

    template<typename T>
    void store_impl(T t, std::true_type&&) {
        static_assert(std::is_pointer<T>::value, "should be a pointer to box::interface");
        t->store(this);
    }

    template<typename T, typename... A>
    void store_args(T t, A... args) {
        store(t);
        store_args(args...);
    }

    void store_args() {
    }

    template<typename T>
    T load() {
        if (_pos >= _str.size()) {
            return T();
        }

        return load_impl<T>(
            typename std::is_pointer<T>::type(),
            typename std::is_base_of<interface, typename std::decay<typename std::remove_pointer<T>::type>::type>::type()
        );
    }

    template<typename T, typename C1, typename C2>
    T load_impl(C1&&, C2&&) {
        size_t used_len = 0;
        T result = special<T>::load(_str, _pos, &used_len);
        _pos += used_len;
        return result;
    }

    template<typename T>
    T load_impl(std::false_type&&, std::true_type&&) {
        auto object = std::make_shared<T>();
        _load_objs.insert(object);
        object->load(this);
        return *object;
    }

    template<typename T>
    T load_impl(std::true_type&&, std::true_type&&) {
        auto object = std::make_shared<typename std::decay<typename std::remove_pointer<T>::type>::type>();
        _load_objs.insert(object);
        object->load(this);
        return object.get();
    }

    template<typename... A>
    void invoke(const std::function<void (A...)>& func) {
        func(load<typename std::decay<A>::type>()...);
    }

    template<typename... A>
    void invoke(void (*func)(A...)) {
        func(load<typename std::decay<A>::type>()...);
    }

    template<typename C, typename... A>
    void invoke(void (C::*func)(A...), C* object) {
        (object->*func)(load<typename std::decay<A>::type>()...);
    }

    template<typename R, typename... A>
    R call(const std::function<R (A...)>& func) {
        return func(load<typename std::decay<A>::type>()...);
    }

    template<typename R, typename... A>
    R call(R (*func)(A...)) {
        return func(load<typename std::decay<A>::type>()...);
    }

    template<typename C, typename R, typename... A>
    R call(R (C::*func)(A...), C* object) {
        return (object->*func)(load<typename std::decay<A>::type>()...);
    }

    const std::string & str() const { return _str; }
    void set_str(const std::string &str) { _str = str; }

    size_t pos() const { return _pos; }
    void set_pos(size_t pos) { _pos = pos; }

    operator bool() const {
        return !_str.empty();
    }

private:
    std::string _str;
    size_t _pos = 0;
    std::unordered_set< std::shared_ptr<interface> > _load_objs;
};

} // box
