#ifndef _fsfs_h_
#define _fsfs_h_

#include "scheduler.h"

class FSFS : public Scheduler {

    Queue<gheith::TCB,InterruptSafeLock> readyQ{};

public:
    FSFS() {}

    bool schedule(gheith::TCB* thread, Source source) {

	    readyQ.add(thread);

	    return true;
    }

    gheith::TCB* getNext(){
	    return readyQ.remove();
    }

    void monitor_add(){
	    readyQ.monitor_add();
    }

    void monitor_remove(){
	    readyQ.monitor_remove();
    }
};

#endif
