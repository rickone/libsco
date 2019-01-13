#pragma once

#include <unordered_map>

namespace asyn {

struct co_status {
    int cid = 0;
    int wid = 0;
    int join_cid = 0;
};

class monitor {
public:
    monitor() = default;
    ~monitor() = default;

    co_status* get_co_status(int cid);
    void on_coroutine_start(int cid, int wid);
    void on_coroutine_stop(int cid);

private:
    std::unordered_map<int, co_status> _co_status;
};

} // asyn
