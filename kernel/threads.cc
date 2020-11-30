#include "debug.h"
#include "smp.h"
#include "debug.h"
#include "config.h"
#include "machine.h"
#include "threads.h"
#include "condition.h"

#include "impls.h"

namespace gheith {
    Atomic<uint32_t> TCB::next_id{0};

    TCB** activeThreads;
    TCB** idleThreads;

    Queue<TCB,InterruptSafeLock> zombies{};

    Scheduler* scheduler = new MLQ{
        11,
        [](uint32_t i){
            return new FSFS{};
        },
        [](TCB* tcb, Source source, uint32_t levels){
            uint32_t& scrap = tcb->scrap;
            int& scrap2 = tcb->scrap2;
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
    };

    TCB* current() {
        auto was = Interrupts::disable();
        TCB* out = activeThreads[SMP::me()];
        Interrupts::restore(was);
        return out;
    }

    void entry() {
        sti();
        current()->doYourThing();
        stop();
    }

    void delete_zombies() {
        while (true) {
            auto it = zombies.remove();
            if (it == nullptr) return;
            delete it;
        }
    }

    void schedule(TCB* tcb, Source source) {
        if (!tcb->isIdle) {
	        scheduler->schedule(tcb,source);
        }
    }

    struct IdleTcb: public TCB {
        IdleTcb(): TCB(kProc, true) {}
        void doYourThing() override {
            Debug::panic("should not call this");
        }
    };

    static class {
        InterruptSafeLock mutex{};
        Condition cv{mutex};
    public:	
        void put(TCB* tcb) {
            LockGuard g{mutex};
            zombies.add(tcb);
            cv.notifyOne();
        }
        void init() {
            new (&mutex) InterruptSafeLock{};
            new (&cv) Condition{mutex};
            thread([this] {
                Debug::printf("starting reaper\n");
                TCB* tcb;
                while (true) {
                {
                    LockGuard g{mutex};
                    while (!(tcb = zombies.remove()))
                    cv.wait();	    
                }
                delete tcb;
                }
            });
        }
    } reaper;

    
};

void threadsInit() {
    using namespace gheith;
    activeThreads = new TCB*[kConfig.totalProcs]();
    idleThreads = new TCB*[kConfig.totalProcs]();

    // swiched to using idle threads in order to discuss in class
    for (unsigned i=0; i<kConfig.totalProcs; i++) {
        idleThreads[i] = new IdleTcb();
        activeThreads[i] = idleThreads[i];
    }

    // the reaper
    reaper.init();
}

void yield(Source source){
    using namespace gheith;
    block(BlockOption::CanReturn,[source](TCB* me) {
        schedule(me,source);
    });
}

void stop() {
    using namespace gheith;

    while(true) {
        block(BlockOption::MustBlock,[](TCB* me) {
            if (!me->isIdle) {
		        reaper.put(me);
            }
        });
        ASSERT(current()->isIdle);
    }
}
