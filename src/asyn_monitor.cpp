#include "asyn_monitor.h"
#include "asyn_master.h"

using namespace asyn;

co_status* monitor::get_co_status(int cid) {
    auto it = _co_status.find(cid);
    if (it == _co_status.end()) {
        return nullptr;
    }
    return &it->second;
}

void monitor::on_coroutine_start(int cid, int wid) {
    co_status status = {.cid = cid, .wid = wid};
    _co_status[cid] = status;
}

void monitor::on_coroutine_stop(int cid) {
    auto status = get_co_status(cid);
    if (status == nullptr) {
        return;
    }

    if (status->join_cid != 0) {
        auto join_status = get_co_status(status->join_cid);
        if (join_status) {
            master::inst()->command_worker(join_status->wid, cmd_resume, status->join_cid);
        }
    }

    _co_status.erase(cid);
}
