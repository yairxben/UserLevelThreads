# User Level Threads
A functional static library, that creates and manages user-level threads.

A potential user will be able to include the library and use it according to the package’s public interface:

The uthreads.h header file.

* __The Threads__

  Initially, a program is comprised of the default main thread, whose ID is 0. All other threads will be explicitly
  created. The maximal number of threads the library should support (including the main thread) is MAX_THREAD_NUM.

* __Thread State Diagram__

  At any given time during the running of the user's program, each of the threads in the program is in one of
  the states shown in the following state diagram. Transitions from state to state occur as a result of calling
  one of the library functions, or from elapsing of time, as explained below.

![image](https://user-images.githubusercontent.com/82065601/208743584-d9bb4539-d8fd-4023-9422-40fd47d07a23.png)

* __Scheduler__

  Round-Robin (RR) scheduling algorithm.
  
  
* __API__


  int uthread_init(int quantum_usecs)

  * _This function initializes the thread library._


  int uthread_spawn(void (*f)(void))

  * _This function creates a new thread._


  int uthread_terminate(int tid)

  * _This function terminates the thread with ID tid and deletes it from all relevant control structures._


  int uthread_block(int tid)

  * _This function blocks the thread with ID tid._


  int uthread_resume(int tid)

  * _This function resumes a blocked thread with ID tid and moves it to the READY state._


  int uthread_mutex_lock()

  * _This function tries to acquire a mutex._


  int uthread_mutex_unlock()

  * _This function releases a mutex._


  int uthread_get_tid()

  * _This function returns the thread ID of the calling thread._


  int uthread_get_total_quantums()

  * _This function returns the total number of quantums that were started since the library was
  initialized, including the current quantum._


  int uthread_get_quantums(int tid)

  * _This function returns the number of quantums the thread with ID tid was in RUNNING state._

