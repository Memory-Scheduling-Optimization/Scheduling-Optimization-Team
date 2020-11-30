#ifndef _mlq_h_
#define _mlq_h_

#include "scheduler.h"
#include "atomic.h"
#include "libk.h"

template<typename X, typename Y>
class MLQ : public Scheduler {

    const uint32_t levels;
    Scheduler** schRay;
    Y getScheduler;
    uint32_t toRemove;
    InterruptSafeLock lock;

public:
    
    MLQ(uint32_t levels, X mapper, Y getScheduler) : levels(levels), getScheduler(getScheduler), toRemove(levels) {
        schRay = new Scheduler*[levels];
        for(uint32_t i = 0; i<levels; i++){
            schRay[i] = mapper(i);
        }
    }

    ~MLQ() {
        for(uint32_t i = 0; i<levels; i++){
            delete schRay[i];
        }
        delete[] schRay;
    }

    bool schedule(gheith::TCB* thread, Source source) {
	    LockGuard g{lock};
        uint32_t level = getScheduler(thread,source,levels);
        toRemove = K::min(toRemove,level);
        return schRay[level]->schedule(thread,source);
    }

    gheith::TCB* getNext(){
        LockGuard g{lock};
	    if(toRemove==levels){
            return nullptr;
        }
        gheith::TCB* next = schRay[toRemove]->getNext();
        for(; toRemove < levels; toRemove++){
            if(!schRay[toRemove]->isEmpty()){
                break;
            }
        }
        return next;
    }

    bool isEmpty(){
        LockGuard g{lock};
        bool isEmpty = true;
        for(uint32_t i = 0; isEmpty && i<levels; i++){
            isEmpty = schRay[i]->isEmpty();
        }
        return isEmpty;
    }
};

#endif
