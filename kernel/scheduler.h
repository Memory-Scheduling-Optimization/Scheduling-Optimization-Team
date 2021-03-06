#ifndef _scheduler_h_
#define _scheduler_h_

#include "atomic.h"
#include "queue.h"
#include "smp.h"
#include "shared.h"
#include "debug.h"
#include "tcb.h"

enum class Source {
    INIT,
    PREEMPT,
    MANUAL
};

struct Scheduler{

    Scheduler() {

    }

    virtual ~Scheduler() {

    }
    
    virtual bool schedule(gheith::TCB*,Source) = 0;

    virtual gheith::TCB* getNext() = 0;

    virtual bool isEmpty() = 0;
};

#endif
