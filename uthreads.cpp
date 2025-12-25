#include "uthreads.h"
#include "TCB.h"
#include "IDGenerator.h"
#include <signal.h>
#include <sys/time.h>
#include <iostream>
#include <algorithm>
#include <queue>

std::deque<TCB*> ready_queue;
std::vector<TCB*> all_threads;
IDGenerator id_gen;
TCB* running_thread;
struct itimerval timer{};
int total_quantum;


void mask() {
    sigset_t new_mask, old_mask;

    // Block SIGINT signal
    sigemptyset(&new_mask);
    sigaddset(&new_mask, SIGVTALRM);
    sigprocmask(SIG_BLOCK, &new_mask, &old_mask);
}

void unmask() {
    sigset_t new_mask, old_mask;

    // Block SIGINT signal
    sigemptyset(&new_mask);
    sigaddset(&new_mask, SIGVTALRM);
    sigprocmask(SIG_UNBLOCK, &new_mask, &old_mask);
}

void clear_threads() {
    for(int i = 0; i < all_threads.size(); i++) {
        all_threads.at(i)->~TCB();
    }

    all_threads.clear();
}


void update_sleep_of_threads() {
    mask();
    for(auto& it : all_threads) {
        if(it->state == TCB::State::SLEEPING_WO_BLOCKED || it->state == TCB::State::BLOCKED_AND_SLEEPING) {
            it->sleep_left--;
            if(it->sleep_left < 1 && it->state == TCB::State::SLEEPING_WO_BLOCKED) {
                it->state = TCB::State::READY;
                ready_queue.push_back(it);
            }
        }
    }
    unmask();
}

// the RR algorithm
void scheduler(int signal) {
    total_quantum++;
    update_sleep_of_threads();

    // Start a virtual timer. It counts down whenever this process is executing.
    if (setitimer(ITIMER_VIRTUAL, &timer, NULL))
    {
        std::cerr << "system error: timer error"  << std::endl;
        clear_threads();
        exit(0);
    }

    if(!ready_queue.empty()) {
        TCB *next_thread = ready_queue.front();
        ready_queue.pop_front();

        // save the previous thread's context
        if (sigsetjmp(running_thread->env, 1) == 0)
        {
            ready_queue.push_back(running_thread);
            running_thread->state = TCB::State::READY;
            next_thread->quantum_num++;
            running_thread = next_thread;
            next_thread->state = TCB::State::RUNNING;

            // jump to the next
            siglongjmp(next_thread->env, 1);
        }

    } else {
        running_thread->quantum_num++;
    }
}


int uthread_init(int quantum_usecs) {
    if(quantum_usecs <= 0) {
        std::cerr << "thread library error: invalid quantum usecs"  << std::endl;
        return -1;
    }

    total_quantum = 1;

    // create main thread
    TCB* main_thread = new TCB(0, nullptr);
    running_thread = main_thread;
    running_thread->quantum_num = 1;
    all_threads.push_back(main_thread);
    id_gen = IDGenerator();

    struct sigaction sa = {0};

    // Install scheduler as the signal handler for SIGVTALRM.
    sa.sa_handler = &scheduler;
    sigset_t block_mask;
    sigemptyset(&block_mask);
    sigaddset(&block_mask, SIGVTALRM);
    sa.sa_mask = block_mask;
    sa.sa_flags = 0;
    if (sigaction(SIGVTALRM, &sa, NULL) < 0)
    {
        std::cerr << "system error: sigaction error"  << std::endl;
        clear_threads();
        exit(0);
    }


    timer.it_value.tv_usec = quantum_usecs;

    // Start a virtual timer. It counts down whenever this process is executing.
    if (setitimer(ITIMER_VIRTUAL, &timer, NULL))
    {
        std::cerr << "system error: timer error"  << std::endl;
        clear_threads();
        exit(0);
    }

    return 0;
}

int uthread_spawn(thread_entry_point entry_point) {
    mask();
    if(entry_point == nullptr) {
        std::cerr << "thread library error: null entry point"  << std::endl;
        return -1;
    }
    if(all_threads.size() == MAX_THREAD_NUM) {
        std::cerr << "thread library error: you reached the max number of threads" << std::endl;
        return -1;
    }
    int tid =id_gen.generateID();
    TCB* new_thread = new TCB(tid, entry_point);
    new_thread->state = TCB::State::READY;
    ready_queue.push_back(new_thread);
    all_threads.push_back(new_thread);

    unmask();
    return tid;
}

TCB* find_tcb_by_id(int tid) {
    mask();
    //auto it = find_if(all_threads.begin(), all_threads.end(), [&tid](TCB *obj) {return obj->tid == tid;});
//    std::vector<TCB*>::iterator it = all_threads.begin();
//    std::vector<TCB*>::iterator found = all_threads.end();
//    while(it != all_threads.end()) {
//        if((*it)->tid == tid) {
//            found = it;
//        }
//        it++;
//    }

    TCB* item = nullptr;
    for(int i = 0; i < (int)all_threads.size(); i++) {
        if(all_threads.at(i)->tid == tid) {
            item = all_threads.at(i);
        }
    }

    unmask();
    return item;

//    if (found == all_threads.end())
//    {
//        return nullptr;
//    }
//    else {
//        return (*found);
//    }
}

int uthread_get_tid() {
    return running_thread->tid;
}

int uthread_get_total_quantums() {
    return total_quantum;
}

int uthread_get_quantums(int tid) {
    TCB* thread = find_tcb_by_id(tid);
    if(thread == nullptr) {
        std::cerr << "thread library error: thread doesn't exists"  << std::endl;
        return -1;
    }
    return thread->quantum_num;
}

int uthread_terminate(int tid) {
    mask();
    if(tid == 0) {
        clear_threads();
        exit(0);
    } else {
        TCB* thread = find_tcb_by_id(tid);
        if(thread == nullptr) {
            std::cerr << "thread library error: thread doesn't exists"  << std::endl;
            return -1;
        }

        if(running_thread->tid == tid) {
            total_quantum++;
            update_sleep_of_threads();
            // schedualing decision
            if(!ready_queue.empty()) {
                TCB *next_thread = ready_queue.front();
                ready_queue.pop_front();

                next_thread->quantum_num++;
                running_thread = next_thread;

                next_thread->state = TCB::State::RUNNING;

                // Start a virtual timer. It counts down whenever this process is executing.
                if (setitimer(ITIMER_VIRTUAL, &timer, NULL))
                {
                    std::cerr << "system error: timer error"  << std::endl;
                    clear_threads();
                    exit(0);
                }

                auto it = find_if(ready_queue.begin(), ready_queue.end(), [&tid](TCB *obj) {return obj->tid == tid;});
                if(it != ready_queue.end()) {
                    ready_queue.erase(it);
                }
                auto it2 = find_if(all_threads.begin(), all_threads.end(), [&tid](TCB *obj) {return obj->tid == tid;});
                if(it2 != all_threads.end()) {
                    (*it2)->~TCB();
                    all_threads.erase(it2);
                }

                id_gen.releaseID(tid);

                unmask();
                siglongjmp(next_thread->env, 1);


            } else {
                running_thread->quantum_num++;
            }
        }
        else {
            auto it = find_if(ready_queue.begin(), ready_queue.end(), [&tid](TCB *obj) {return obj->tid == tid;});
            if(it != ready_queue.end()) {
                ready_queue.erase(it);
            }
            auto it2 = find_if(all_threads.begin(), all_threads.end(), [&tid](TCB *obj) {return obj->tid == tid;});
            if(it2 != all_threads.end()) {
                (*it2)->~TCB();
                all_threads.erase(it2);
            }
            id_gen.releaseID(tid);
        }
    }

    unmask();

    return 0;
}

int uthread_block(int tid) {
    mask();
    TCB* thread = find_tcb_by_id(tid);
    if(thread == nullptr || tid == 0) {
        std::cerr << "thread library error: can't block main thread or thread doesn't exist"  << std::endl;
        return -1;
    }

    if(thread->state == TCB::State::SLEEPING_WO_BLOCKED) {
        thread->state = TCB::State::BLOCKED_AND_SLEEPING;
    }
    else {
        thread->state = TCB::State::BLOCKED_WO_SLEEPING;
    }

    if(running_thread->tid == tid) {
        // schedualing decision
        total_quantum++;
        update_sleep_of_threads();
        if(!ready_queue.empty()) {
            TCB *next_thread = ready_queue.front();
            ready_queue.pop_front();

            if (sigsetjmp(running_thread->env, 1) == 0)
            {
                next_thread->quantum_num++;
                running_thread = next_thread;
                next_thread->state = TCB::State::RUNNING;

                // Start a virtual timer. It counts down whenever this process is executing.
                if (setitimer(ITIMER_VIRTUAL, &timer, NULL))
                {
                    std::cerr << "system error: timer error"  << std::endl;
                    clear_threads();
                    exit(0);
                }
                auto it = find_if(ready_queue.begin(), ready_queue.end(), [&tid](TCB *obj) {return obj->tid == tid;});
                if(it != ready_queue.end()) {
                    ready_queue.erase(it);
                }


                unmask();
                siglongjmp(next_thread->env, 1);
            }
        }
        else {
            running_thread->quantum_num++;
        }
    }
    else {
        auto it = find_if(ready_queue.begin(), ready_queue.end(), [&tid](TCB *obj) {return obj->tid == tid;});
        if(it != ready_queue.end()) {
            ready_queue.erase(it);
        }
    }

    unmask();
    return 0;
}

int uthread_resume(int tid) {
    mask();
    TCB* thread = find_tcb_by_id(tid);
    if(thread == nullptr) {
        std::cerr << "thread library error: thread doesn't exist"  << std::endl;
        return -1;
    }

    if(running_thread == thread || tid == 0 || thread->state == TCB::READY) {
        // nothing
    }
    else if(thread->state == TCB::State::BLOCKED_WO_SLEEPING) {
        thread->state = TCB::State::READY;
        ready_queue.push_back(thread);
    }
    else if(thread->state == TCB::State::BLOCKED_AND_SLEEPING) {
        thread->state = TCB::State::SLEEPING_WO_BLOCKED;
    }

    unmask();
    return 0;
}

int uthread_sleep(int num_quantums) {
    mask();
    total_quantum++;
    update_sleep_of_threads();
    int tid = running_thread->tid;
    running_thread->state = TCB::State::SLEEPING_WO_BLOCKED;
    running_thread->sleep_left = num_quantums;

    // schedualing decision
    if(!ready_queue.empty()) {
        TCB *next_thread = ready_queue.front();
        ready_queue.pop_front();

        if (sigsetjmp(running_thread->env, 1) == 0)
        {
            next_thread->quantum_num++;
            next_thread->state = TCB::State::RUNNING;
            running_thread = next_thread;

            // Start a virtual timer. It counts down whenever this process is executing.
            if (setitimer(ITIMER_VIRTUAL, &timer, NULL))
            {
                std::cerr << "system error: timer error"  << std::endl;
                clear_threads();
                exit(0);
            }
            auto it = find_if(ready_queue.begin(), ready_queue.end(), [&tid](TCB *obj) {return obj->tid == tid;});
            if(it != ready_queue.end()) {
                ready_queue.erase(it);
            }

            unmask();
            siglongjmp(next_thread->env, 1);
        }
    }
    else {
        running_thread->quantum_num++;
    }

    unmask();
    return 0;
}