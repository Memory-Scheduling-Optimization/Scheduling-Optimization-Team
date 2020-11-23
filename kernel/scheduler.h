#ifndef _scheduler_h_
#define _scheduler_h_

#include "atomic.h"
#include "queue.h"
#include "smp.h"
#include "shared.h"
#include "debug.h"

template <typename T>
class Scheduler{

    Queue<T,InterruptSafeLock> readyQ{};

public:
    Scheduler() {}

    bool schedule(T* thread, int source) {

	readyQ.add(thread);
	
	return 1;
    }

    T* getNext(){

	return readyQ.remove();
    }

    //Used by geith to circumvent some concurrency issues
    //I think
    
    void monitor_add(){
	readyQ.monitor_add();
    }

    void monitor_remove(){
	readyQ.monitor_remove();
    }
};

#endif
