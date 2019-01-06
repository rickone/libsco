#pragma once

#include <pthread.h>

namespace asy {

class scheduler;

class executer {
public:
    executer() = default;
    virtual ~executer() = default;

    void run(scheduler* sche);
    void join();
    void on_exec();

private:
    pthread_t _thread = nullptr;
    scheduler* _sche = nullptr;
};

} // asy
