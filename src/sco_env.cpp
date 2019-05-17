#include "sco_env.h"
#include <unistd.h>
#include <cstring>
#include <mutex>

using namespace sco;

env* env::inst() {
    static std::once_flag s_once;
    static env s_inst;

    std::call_once(s_once, std::bind(&env::init, &s_inst));
    return &s_inst;
}

extern char** environ;

void env::init() {
    for (char** env = environ; *env; env++) {
        char* p = strstr(*env, "=");
        if (!p) {
            continue;
        }

        size_t len = p - *env;
        std::string key(*env, len);
        std::string value(*env + len + 1);
        _dict.emplace(key, value);
    }

    int flat_ns = get_env_int("DYLD_FORCE_FLAT_NAMESPACE");
    if (!flat_ns) {
        fprintf(stderr, "DYLD_FORCE_FLAT_NAMESPACE not set!\n");
    }
}

std::string env::get_env(const char* name) {
    auto it = _dict.find(name);
    if (it == _dict.end()) {
        return "";
    }

    return it->second;
}

int env::get_env_int(const char* name) {
    auto value = get_env(name);
    if (value == "") {
        return 0;
    }

    return std::stoi(value, nullptr);
}

bool env::get_env_bool(const char* name) {
    auto value = get_env(name);

    auto it = value.begin();
    auto it_end = value.end();
    while (it != it_end) {
        *it = (char)std::tolower(*it);
        ++it;
    }

    if (value == "" || value == "0" || value == "off" || value == "false") {
        return false;
    }

    return true;
}
