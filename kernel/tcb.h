#ifndef _tcb_h_
#define _tcb_h_

#include "pcb.h"

namespace gheith {
    struct TCB;

    struct SaveArea {
        uint32_t ebx;
        uint32_t esp;
        uint32_t ebp;
        uint32_t esi;
        uint32_t edi;
        uint32_t cr2;
        volatile uint32_t no_preempt;
        TCB* tcb;
    };

    struct TCB {
        static Atomic<uint32_t> next_id;

        const bool isIdle;
        const uint32_t id;

        TCB* next;
        SaveArea saveArea;
        Shared<PCB> pcb;
        void* scrap = nullptr;
        uint32_t core;

        TCB(bool isIdle) : isIdle(isIdle), id(next_id.fetch_add(1)) {
            saveArea.tcb = this;
        }

        TCB(Shared<PCB> pcb, bool isIdle) : isIdle(isIdle), id(next_id.fetch_add(1)), pcb(pcb) {
            saveArea.tcb = this;
        }

        virtual ~TCB() {
            if(scrap != nullptr){
                free(scrap);
            }
        }

        virtual void doYourThing() = 0;
    };
}

#endif
