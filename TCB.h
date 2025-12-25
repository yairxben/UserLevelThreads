#ifdef __x86_64__
/* code for 64 bit Intel arch */

typedef unsigned long address_t;
#define JB_SP 6
#define JB_PC 7

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%fs:0x30,%0\n"
                 "rol    $0x11,%0\n"
            : "=g" (ret)
            : "0" (addr));
    return ret;
}

#else
/* code for 32 bit Intel arch */

typedef unsigned int address_t;
#define JB_SP 4
#define JB_PC 5


/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%gs:0x18,%0\n"
                 "rol    $0x9,%0\n"
    : "=g" (ret)
    : "0" (addr));
    return ret;
}
#endif

#include <setjmp.h>
#include <signal.h>
#include <iostream>
#include "uthreads.h"

class TCB {

public:
    TCB(int tid, thread_entry_point entry_point) {
        this->tid = tid;
        this->sleep_left = 0;
        this->quantum_num = 0;
        this->entry_point = entry_point;
        this->stack_p = new char[STACK_SIZE];

        // init env
        address_t sp = (address_t) this->stack_p + STACK_SIZE - sizeof(address_t);
        address_t pc = (address_t) entry_point;
        sigsetjmp(env, 1);
        (env->__jmpbuf)[JB_SP] = translate_address(sp);
        (env->__jmpbuf)[JB_PC] = translate_address(pc);
        sigemptyset(&env->__saved_mask);
    }

    ~TCB() {
        delete[] this->stack_p;
    }


    enum State {READY, BLOCKED_WO_SLEEPING, BLOCKED_AND_SLEEPING, SLEEPING_WO_BLOCKED, RUNNING};

    int tid;
    int quantum_num;
    int sleep_left;
    sigjmp_buf env;
    thread_entry_point entry_point;
    State state;
    char * stack_p;

};
