#pragma once

#include <unordered_map>
#include <queue>
#include "box.h"
#include "asyn_lockfree_queue.h"

namespace asyn {

enum {
    COSTATUS_DEAD,
    COSTATUS_RUNNING,
    COSTATUS_SUSPEND,
};

struct co_snapshot {
    int id = 0;
    int wid = -1;
    int resume_cid = 0;
    int status = COSTATUS_DEAD;
    std::string data;
};

struct mutex_snapshot {
    int id = 0;
    int lock_cid = 0;
    std::queue<int> wait_queue;
};

class coordinator {
public:
    coordinator() = default;
    ~coordinator() = default;

    static coordinator* inst();

    void on_step();
    co_snapshot* get_co_snapshot(int cid);
    mutex_snapshot* get_mutex_snapshot(int mid);
    void on_request(int type, box::object& obj);
    void on_create(int cid);
    void on_start(int cid, int wid);
    void on_resume(int cid, int tcid, const std::string& str);
    void on_yield(int cid, const std::string& str);
    void on_return(int cid, const std::string& str);
    void on_lock(int cid, int mid);
    void on_unlock(int cid, int mid);

    template<typename... A>
    void request(int type, A... args) {
        box::object obj;
        obj.store(type);
        obj.store_args(args...);
        _requests.push(std::move(obj));
    }

private:
    lockfree_queue<box::object> _requests;
    std::unordered_map<int, co_snapshot> _co_snapshot_dict;
    std::unordered_map<int, mutex_snapshot> _mutex_snapshot_dict;
};

enum { // request type
    req_create,
    req_start,
    req_resume,
    req_yield,
    req_return,
    req_lock,
    req_unlock,
};

} // asyn
