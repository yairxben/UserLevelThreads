# User Level Threads Library

## Project Description
This project is a functional static library implementing a **User-Level Thread (ULT)** management system. It enables creating, scheduling, and synchronizing threads entirely in user space. The library manages the full thread lifecycle—from initialization to termination—using a **Round-Robin (RR) scheduling algorithm** to ensure fair CPU time distribution.

## Motivation
User-level threads offer significantly faster context switching by avoiding kernel system calls. This library allows users to:
* Implement **high-performance concurrency** with minimal overhead.
* Control **custom scheduling** policies by defining specific time quantums.
* Gain deep insights into low-level OS mechanics like context switching, task scheduling, and mutex synchronization.

## Thread State Diagram
The library manages transitions between **RUNNING**, **READY**, and **BLOCKED** states as shown below:

![Thread State Diagram](https://user-images.githubusercontent.com/82065601/208743584-d9bb4539-d8fd-4023-9422-40fd47d07a23.png)

## API Overview
The library exposes a simple C interface (`uthreads.h`) for managing threads:

* **Management:** `uthread_spawn` (create), `uthread_terminate` (delete), `uthread_init`.
* **Control:** `uthread_block` (pause), `uthread_resume` (unpause).
* **Synchronization:** `uthread_mutex_lock`, `uthread_mutex_unlock`.
* **Monitoring:** `uthread_get_tid`, `uthread_get_total_quantums`, `uthread_get_quantums`.
