#pragma once

#include <pthread.h>
#include "asy_coroutine.h"
#include "asy_queue.h"

namespace asy {

enum {
    exe_test,
};

class scheduler;

class executer {
public:
    executer() = default;
    virtual ~executer() = default;

    void run(int id, scheduler* sche);
    void join();
    void on_exec();
    void on_request(int type, box::object& obj);
    void on_test(int a, int b);

    template<typename... A>
    void request(int type, A... args) {
        box::object obj;
        obj.store(type);
        obj.store(args...);
        _requests.push(std::move(obj));
    }

private:
    int _id = 0;
    scheduler* _sche = nullptr;
    pthread_t _thread = nullptr;
    std::list<std::shared_ptr<coroutine>> _coroutine_list;
    queue<box::object> _requests;
};

} // asy
