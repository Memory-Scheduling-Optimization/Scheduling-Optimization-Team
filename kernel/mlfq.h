#ifndef _mlfq_h_
#define _mlfq_h_

#include "mlq.h"

template<typename F>
class MLFQ : public MLQ<F> {

    uint32_t getScheduler(gheith::TCB* thread, Source source) {
        uint32_t& scrap = thread->scrap;
        int& scrap2 = thread->scrap2;
        uint32_t levels = MLQ<F>::levels;
        constexpr int s2Up = 3;
        constexpr int s2Down = -3;
        switch(source){
            case Source::INIT:
                scrap = levels/2;
                scrap2 = 0;
                break;
            case Source::MANUAL:
                scrap2 = (scrap2>s2Down)?scrap2-1:scrap2;
                break;
            case Source::PREEMPT:
                scrap2 = (scrap2<s2Up)?scrap2+1:scrap2;
                break;
        }
        if(scrap > 0 && scrap2 == s2Down){
            scrap--;
            scrap2 = 0;
        }else if(scrap < levels-1 && scrap2 == s2Up){
            scrap++;
            scrap2 = 0;
        }
        return scrap;
    }

public:
    
    MLFQ(uint32_t levels, F mapper) : MLQ<F>(levels,mapper) {}

};

#endif
