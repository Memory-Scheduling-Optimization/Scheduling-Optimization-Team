#ifndef _threads_h_
#define _threads_h_

#include "atomic.h"
#include "queue.h"
#include "heap.h"
#include "debug.h"
#include "smp.h"
#include "shared.h"
#include "new.h"
#include "scheduler.h"

#include "tss.h"
#include "pcb.h"
#include "tcb.h"

namespace gheith {

    constexpr static int STACK_BYTES = 8 * 1024;
    constexpr static int STACK_WORDS = STACK_BYTES / sizeof(uint32_t);

    extern "C" void gheith_contextSwitch(gheith::SaveArea*, gheith::SaveArea*, void* action, void* arg);

    extern TCB** activeThreads;
    extern TCB** idleThreads;

    extern TCB* current();
    extern Scheduler* scheduler;
    extern void entry();
    extern void schedule(TCB*,Source=Source::MANUAL);
    extern void delete_zombies();

    template <typename F>
    void caller(SaveArea* sa, F* f) {
        (*f)(sa->tcb);
    }
    
    // There is a bit of template/lambda voodoo here that allows us to
    // call block like this:
    //
    //   block([](TCB* previousTCB) {
    //        // do any cleanup work here but remember that you are:
    //        //     - running on the target stack
    //        //     - interrupts are disabled so you better be quick
    //   }

    enum class BlockOption {
        MustBlock,
        CanReturn
    };

    template <typename F>
    void block(BlockOption blockOption, const F& f) {

        uint32_t core_id;
        TCB* me;
        bool before;

        Interrupts::protect([&core_id,&me,&before] {
            core_id = SMP::me();
            me = activeThreads[core_id];
            before = me->saveArea.no_preempt;
            me->saveArea.no_preempt = true;
        });
        
    again:
	    auto next_tcb = scheduler->getNext();
        if (next_tcb == nullptr) {
            if (blockOption == BlockOption::CanReturn) {
                me->saveArea.no_preempt = before;
                return;
            }
            if (me->isIdle) {
                // Many students had problems with hopping idle threads
                ASSERT(core_id == SMP::me());
                ASSERT(!Interrupts::isDisabled());
                ASSERT(me == idleThreads[core_id]);
                ASSERT(me == activeThreads[core_id]);
                iAmStuckInALoop(false);
                goto again;
            }
            next_tcb = idleThreads[core_id];
        }

        next_tcb->saveArea.no_preempt = true;

        activeThreads[core_id] = next_tcb;  // Why is this safe?
        setCR3(next_tcb->pcb->cr3);
        tss[core_id].esp0 = next_tcb->pcb->esp0;

        gheith_contextSwitch(&me->saveArea,&next_tcb->saveArea,(void*)caller<F>,(void*)&f);

        me->saveArea.no_preempt = before;
    }

    struct TCBWithStack : public TCB {
        uint32_t *stack = new uint32_t[STACK_WORDS];
    
        TCBWithStack() : TCB(false) {
            stack[STACK_WORDS - 2] = 0x200;  // EFLAGS: IF
            stack[STACK_WORDS - 1] = (uint32_t) entry;
	        saveArea.no_preempt = 0;
            saveArea.esp = (uint32_t) &stack[STACK_WORDS-2];
        }

        TCBWithStack(Shared<PCB> pcb) : TCB(pcb, false) {
            stack[STACK_WORDS - 2] = 0x200;  // EFLAGS: IF
                stack[STACK_WORDS - 1] = (uint32_t) entry;
                saveArea.no_preempt = false;
                saveArea.esp = (uint32_t) &stack[STACK_WORDS-2];
            pcb->esp0 = saveArea.esp;
        }
	
        ~TCBWithStack() {
            if (stack) {
                delete[] stack;
                stack = nullptr;
            }
        }
    };
    

    template <typename T>
    struct TCBImpl : public TCBWithStack {
        T work;

        TCBImpl(T work) : TCBWithStack(), work(work) {}
	
	    TCBImpl(Shared<PCB> pcb, T work) : TCBWithStack(pcb), work(work) {}
	
        ~TCBImpl() {}

        void doYourThing() override {
            saveArea.no_preempt = false;
            work();
        }
    };

};

extern void threadsInit();

extern void stop();
extern void yield(Source source=Source::MANUAL);

template <typename T>
void thread(T work, Shared<PCB> pcb=kProc) {
    using namespace gheith;

    delete_zombies();

    auto tcb = new TCBImpl<T>(pcb, work);
    schedule(tcb,Source::INIT);
}


#endif
