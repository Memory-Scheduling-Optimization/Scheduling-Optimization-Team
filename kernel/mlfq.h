#ifndef _mlfq_h_
#define _mlfq_h_

#include "mlq.h"

struct MLFQScrap {
    uint32_t priority;
    int progress;
};

template<typename F>
class MLFQ : public MLQ<F> {

    uint32_t getScheduler(gheith::TCB* thread, Source source) {
        MLFQScrap* scrap = (MLFQScrap*)thread->scrap;
        uint32_t levels = MLQ<F>::levels;
        constexpr int upGoal = 3;
        constexpr int downGoal = -3;
        switch(source){
            case Source::INIT:
                scrap = new MLFQScrap{};
                scrap->priority = levels/2;
                scrap->progress = 0;
                thread->scrap = scrap;
                break;
            case Source::MANUAL:
                scrap->progress = (scrap->progress>downGoal)?scrap->progress-1:scrap->progress;
                break;
            case Source::PREEMPT:
                scrap->progress = (scrap->progress<upGoal)?scrap->progress+1:scrap->progress;
                break;
        }
        uint32_t& priority = scrap->priority;
        int& progress = scrap->progress;
        if(priority > 0 && progress == downGoal){
            priority--;
            progress = 0;
        }else if(priority < levels-1 && progress == upGoal){
            priority++;
            progress = 0;
        }
        return priority;
    }

public:
    
    MLFQ(uint32_t levels, F mapper) : MLQ<F>(levels,mapper) {}

};

#endif
