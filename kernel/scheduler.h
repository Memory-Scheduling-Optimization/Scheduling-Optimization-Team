#ifndef _scheduler_h_
#define _scheduler_h_

#include "atomic.h"
#include "queue.h"
#include "smp.h"
#include "shared.h"
#include "debug.h"
#include "tcb.h"

enum class Source {
    PREEMPT,
    MANUAL
};

struct Scheduler{

    Scheduler() {

    }

    virtual ~Scheduler() {

    }

    virtual bool schedule(gheith::TCB* thread, Source source) = 0;

    virtual gheith::TCB* getNext() = 0;
    
    // Can we always implement monitor? Seems difficult for a multilevel queue

    virtual void monitor_add() = 0;

    virtual void monitor_remove() = 0;
};

#endif
