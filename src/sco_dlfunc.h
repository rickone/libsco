#pragma once

#include <dlfcn.h>
#include <string>
#include <functional>
#include "sco_except.h"

namespace sco {

class dlfunc {
public:
    dlfunc() {
    }

    dlfunc(const char* libname, const char* funcname) {
        open(libname);
        sym(funcname);
    }

    ~dlfunc() {
        close();
    }

    void close() {
        if (_handler) {
            dlclose(_handler);
        }
    }

    void open(const char* libname) {
        std::string str_name("lib");
        str_name.append(libname);
    #ifdef __APPLE__
        str_name.append(".dylib");
    #endif
    #ifdef __linux__
        str_name.append(".so");
    #endif

        close();
        _handler = dlopen(str_name.c_str(), RTLD_LAZY);
        runtime_assert(_handler, "dlopen('%s') failed: %s", str_name.c_str(), dlerror());
    }

    void sym(const char* funcname) {
        runtime_assert(_handler, "");

        _addr = dlsym(_handler, funcname);
        runtime_assert(_addr, "dlsym('%s') failed: %s", funcname, dlerror());
    }

    dlfunc& next(const char* funcname) {
        if (_addr) {
            return *this;
        }

        _addr = dlsym(RTLD_NEXT, funcname);
        runtime_assert(_addr, "dlsym(RTLD_NEXT, '%s') failed: %s", funcname, dlerror());

        printf("RTLD_NEXT %s\n", funcname);
        return *this;
    }

    template<typename R, typename... A>
    auto operator()(R (*func)(A...)) {
        return (decltype(func))_addr;
    }

private:
    void* _handler = nullptr;
    void* _addr = nullptr;
};

#define sys_hook(func) static sco::dlfunc s_##func;
#define sys_org(func) s_##func.next(#func)(func)

} // sco
