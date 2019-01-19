#include "asyn_coordinator.h"
#include "asyn_master.h"

using namespace asyn;

coordinator* coordinator::inst() {
    static coordinator s_inst;
    return &s_inst;
}

void coordinator::on_step() {
    while (true) {
        box::object obj;
        if (!_requests.pop(obj)) {
            break;
        }

        auto type = obj.load<int>();
        on_request(type, obj);
    }
}

co_snapshot* coordinator::get_co_snapshot(int cid) {
    auto it = _co_snapshot_dict.find(cid);
    if (it == _co_snapshot_dict.end()) {
        co_snapshot obj = {.id = cid};
        auto ret = _co_snapshot_dict.emplace(cid, obj);
        it = ret.first;
    }
    return &it->second;
}

mutex_snapshot* coordinator::get_mutex_snapshot(int mid) {
    auto it = _mutex_snapshot_dict.find(mid);
    if (it == _mutex_snapshot_dict.end()) {
        mutex_snapshot obj = {.id = mid};
        auto ret = _mutex_snapshot_dict.emplace(mid, obj);
        it = ret.first;
    }
    return &it->second;
}

void coordinator::on_request(int type, box::object& obj) {
    //printf("on_request: %d\n", type);
    switch (type) {
        case req_create:
            obj.invoke(&coordinator::on_create, this);
            break;
        case req_start:
            obj.invoke(&coordinator::on_start, this);
            break;
        case req_resume:
            obj.invoke(&coordinator::on_resume, this);
            break;
        case req_yield:
            obj.invoke(&coordinator::on_yield, this);
            break;
        case req_return:
            obj.invoke(&coordinator::on_return, this);
            break;
        case req_lock:
            obj.invoke(&coordinator::on_lock, this);
            break;
        case req_unlock:
            obj.invoke(&coordinator::on_unlock, this);
            break;
    }
}

void coordinator::on_create(int cid) {
    auto co_snapshot = get_co_snapshot(cid);
    co_snapshot->status = COSTATUS_RUNNING;
}

void coordinator::on_start(int cid, int wid) {
    auto co_snapshot = get_co_snapshot(cid);
    co_snapshot->wid = wid;
}

void coordinator::on_resume(int cid, int target_cid, const std::string& str) {
    auto co_snapshot = get_co_snapshot(cid);
    auto target_co_snapshot = get_co_snapshot(target_cid);

    if (target_co_snapshot->status == COSTATUS_DEAD) {
        master::inst()->command_worker(co_snapshot->wid, cmd_resume, cid, target_co_snapshot->data);
        _co_snapshot_dict.erase(target_cid);
        return;
    }

    if (target_co_snapshot->status == COSTATUS_RUNNING) {
        target_co_snapshot->resume_cid = cid;
        target_co_snapshot->data = str;
        return;
    }

    master::inst()->command_worker(co_snapshot->wid, cmd_resume, cid, target_co_snapshot->data);
    master::inst()->command_worker(target_co_snapshot->wid, cmd_resume, target_cid, str);
    target_co_snapshot->status = COSTATUS_RUNNING;
}

void coordinator::on_yield(int cid, const std::string& str) {
    auto co_snapshot = get_co_snapshot(cid);
    if (co_snapshot->resume_cid == 0) {
        co_snapshot->status = COSTATUS_SUSPEND;
        co_snapshot->data = str;
        return;
    }

    auto resume_status = get_co_snapshot(co_snapshot->resume_cid);
    master::inst()->command_worker(co_snapshot->wid, cmd_resume, cid, co_snapshot->data);
    master::inst()->command_worker(resume_status->wid, cmd_resume, co_snapshot->resume_cid, str);
    co_snapshot->resume_cid = 0;
}

void coordinator::on_return(int cid, const std::string& str) {
    auto co_snapshot = get_co_snapshot(cid);
    if (co_snapshot->status == COSTATUS_DEAD) {
        return;
    }

    if (co_snapshot->resume_cid == 0) {
        co_snapshot->status = COSTATUS_DEAD;
        co_snapshot->data = str;
        return;
    }

    auto resume_co_snapshot = get_co_snapshot(co_snapshot->resume_cid);
    master::inst()->command_worker(resume_co_snapshot->wid, cmd_resume, co_snapshot->resume_cid, str);
    _co_snapshot_dict.erase(cid);
}

void coordinator::on_lock(int cid, int mid) {
    auto mutex_snapshot = get_mutex_snapshot(mid);
    if (mutex_snapshot->lock_cid != 0) {
        mutex_snapshot->wait_queue.push(cid);
        return;
    }

    auto co_snapshot = get_co_snapshot(cid);
    mutex_snapshot->lock_cid = cid;
    master::inst()->command_worker(co_snapshot->wid, cmd_resume, cid);
}

void coordinator::on_unlock(int cid, int mid) {
    auto mutex_snapshot = get_mutex_snapshot(mid);
    if (mutex_snapshot->lock_cid != cid) { // panic
        return;
    }

    mutex_snapshot->lock_cid = 0;
    if (mutex_snapshot->wait_queue.empty()) {
        return;
    }

    int wait_cid = mutex_snapshot->wait_queue.front();
    mutex_snapshot->wait_queue.pop();

    auto co_snapshot = get_co_snapshot(wait_cid);
    mutex_snapshot->lock_cid = wait_cid;
    master::inst()->command_worker(co_snapshot->wid, cmd_resume, wait_cid);
}
