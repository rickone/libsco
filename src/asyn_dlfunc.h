#pragma once

#include <dlfcn.h>
#include <string>

namespace asyn {

class dlfunc {
public:
    dlfunc(const char* libname, const char* funcname) {
        std::string str_name("lib");
        str_name.append(libname);
#ifdef __APPLE__
        str_name.append(".dylib");
#endif
#ifdef __linux__
        str_name.append(".so");
#endif

        _handler = dlopen(str_name.c_str(), RTLD_NOW);
        if (_handler == nullptr) { // panic
            fprintf(stderr, "dlopen failed: %s\n", dlerror());
            return;
        }

        _addr = dlsym(_handler, funcname);
        if (_addr == nullptr) { // panic
            fprintf(stderr, "dlsym failed: %s\n", dlerror());
        }
    }

    ~dlfunc() {
        if (_handler) {
            dlclose(_handler);
        }
    }

    template<typename R, typename... A>
    std::function<R (A...)> operator()(R (*sign)(A...)) {
        typedef decltype(sign) func_t;
        std::function<R (A...)> func = [this](A... args) -> R {
            return ((func_t)_addr)(args...);
        };
        return func;
    }

private:
    void* _handler = nullptr;
    void* _addr = nullptr;
};

} // asyn
