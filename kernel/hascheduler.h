#ifndef _hascheduler_h_
#define _hascheduler_h_

#include "scheduler.h"

class HardAffinityScheduler : public Scheduler {
    // which core to schedule the next thread on (since we can't accurately check the number on each core)
    uint32_t scheduleCore = 0;
    // per core ready queues
    Queue<gheith::TCB,InterruptSafeLock>** queues;

public:

    HardAffinityScheduler() {
        // initialize a queue per core
        queues = new Queue<gheith::TCB,InterruptSafeLock>*[kConfig.totalProcs]();
        for (uint32_t i = 0; i < kConfig.totalProcs; i++) {
            Queue<gheith::TCB,InterruptSafeLock>* readyQ = new Queue<gheith::TCB,InterruptSafeLock>();
            queues[i] = readyQ;
        }
    }

    ~HardAffinityScheduler() {
        delete queues;
    }

    bool schedule(gheith::TCB* thread, Source source) {
        uint32_t core = 0;
        if (source == Source::INIT) {
            // this is a new thread, put it on the next scheduled core
            core = scheduleCore;
            // advance to next core
            scheduleCore = (scheduleCore + 1) % kConfig.totalProcs;
        } else {
            // this is not a new thread, keep running on the same core
            core = SMP::me();
        }
        queues[core]->add(thread);
        return true;
    }

    gheith::TCB* getNext() {
        // get next thread from current core queue
        uint32_t core = SMP::me();
        gheith::TCB* next = queues[core]->remove();
        return next;
    }
};

#endif