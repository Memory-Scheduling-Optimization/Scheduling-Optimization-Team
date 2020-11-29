#ifndef _fsfs_h_
#define _fsfs_h_

#include "scheduler.h"

class FSFS : public Scheduler {

    Queue<gheith::TCB,InterruptSafeLock> readyQ{};

public:
    FSFS() {}

    ~FSFS() {}

    bool schedule(gheith::TCB* thread, Source source) {
	    readyQ.add(thread);
	    return true;
    }

    gheith::TCB* getNext(){
	    return readyQ.remove();
    }
};

#endif
