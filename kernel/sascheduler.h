#ifndef _sascheduler_h_
#define _sascheduler_h_

#include "scheduler.h"

class SoftAffinityScheduler : public Scheduler {
    // which core to schedule the next thread on (since we can't accurately check the number on each core)
    uint32_t scheduleCore = 0;
    // per core ready queues
    Queue<gheith::TCB,InterruptSafeLock>** queues;

public:

    SoftAffinityScheduler() {
        // initialize a queue per core
        queues = new Queue<gheith::TCB,InterruptSafeLock>*[kConfig.totalProcs]();
        for (uint32_t i = 0; i < kConfig.totalProcs; i++) {
            Queue<gheith::TCB,InterruptSafeLock>* readyQ = new Queue<gheith::TCB,InterruptSafeLock>();
            queues[i] = readyQ;
        }
    }

    ~SoftAffinityScheduler() {
        delete queues;
    }

    bool schedule(gheith::TCB* thread, Source source) {
        uint32_t core = 0;
        if (source == Source::INIT) {
            // this is a new thread, put it on the next scheduled core
            core = scheduleCore;
            thread->core = scheduleCore;
            // advance to next core
            scheduleCore = (scheduleCore + 1) % kConfig.totalProcs;
        } else {
            // this is not a new thread, keep running on the same core
            core = thread->core;
        }
        queues[core]->add(thread);
        return true;
    }

    gheith::TCB* getNext() {
        uint32_t core = SMP::me();
        // get next thread from current core queue
        gheith::TCB* next = queues[core]->remove();
        if (next == nullptr) {
            // try to steal a thread from another core's queue
            next = migrate(core);
        }
        return next;
    }

    // pull migration
    gheith::TCB* migrate(uint32_t me) {
        gheith::TCB* stolenThread;
        for (uint32_t i = 0; i < kConfig.totalProcs; i++) {
            // only check this core if it's not me
            if (i != me) {
                stolenThread = queues[i]->remove();
                if (stolenThread != nullptr) return stolenThread;
            }
        }
        return nullptr;
    }
    
    bool isEmpty() {
        return false;
    }
};

#endif
