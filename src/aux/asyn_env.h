#pragma once

#include <string>
#include <unordered_map>

namespace asyn {

class env {
public:
    env() = default;
    ~env() = default;

    static env* inst();

    void init();
    std::string get_env(const char* name);
    int get_env_int(const char* name);
    bool get_env_bool(const char* name);

private:
    std::unordered_map<std::string, std::string> _dict;
};

}
