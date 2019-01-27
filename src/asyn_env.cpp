#include "asyn_env.h"
#include <unistd.h>

using namespace asyn;

env* env::inst() {
    static env s_inst;
    return &s_inst;
}

extern char** environ;

void env::init() {
    //puts("env=");
    for (char** env = environ; *env; env++) {
        char* p = strstr(*env, "=");
        if (!p) {
            continue;
        }

        size_t len = p - *env;
        std::string key(*env, len);
        std::string value(*env + len + 1);
        _dict.emplace(key, value);
        //printf("%s=%s\n", key.c_str(), value.c_str());
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
