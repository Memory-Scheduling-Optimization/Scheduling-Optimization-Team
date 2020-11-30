#ifndef _fcfs_h_
#define _fcfs_h_

#include "scheduler.h"

class FCFS : public Scheduler {

    Queue<gheith::TCB,InterruptSafeLock> readyQ{};

public:
    FCFS() {}

    ~FCFS() {}

    bool schedule(gheith::TCB* thread, Source source) {
	    readyQ.add(thread);
	    return true;
    }

    gheith::TCB* getNext(){
	    return readyQ.remove();
    }

    bool isEmpty(){
        return readyQ.isEmpty();
    }
};

#endif
