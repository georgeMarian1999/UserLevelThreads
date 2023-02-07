#ifndef USERLEVELTHREADS_LIBRARY_H
#define USERLEVELTHREADS_LIBRARY_H


#include <stdio.h>
#include <ucontext.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <malloc.h>
#include <signal.h>


typedef struct {
    int id;
    ucontext_t ctx;
    ucontext_t end_ctx;
    int status; // -1 - not created; 0 - ready; 1 - finished;
    int waitsFor; // -2 - waits for no one; id - id of the thread that he waits for

    int wants_mutex_lock_id; // -1 not wanting to lock any mutex; id of the mutex that the thread wants to lock;
    int wants_mutex_unlock_id; // -1 not wanting to unlock any mutex; id of the mutex that the thread wants to unlock;

    int at_barrier; // -1 not waiting at any barrier; id of the barrier;
} ult;


typedef struct {
    int id;
    int locked_by; // -1 not locked by any thread; id of the thread that holds the lock;
    int status; // -1 - not init; 0 - created
} mutex;

typedef struct {
    int id;
    int capacity; // Number of threads; -1 - barrier is not created else number of threads
} barrier;

void start();
void stop();

void print_array();
void init_array();

void scheduler();

int ult_self(void);
void ult_init(long period);
int ult_create(int created_id, void (*function)(void*), void* arg);
void ult_join(int thread_to_wait_for_id);
void ult_exit();


void mutex_init(int mutex_id);
int mutex_lock(int mutex_id);
int mutex_unlock(int mutex_id);

void barrier_init(int barrier_id, int capacity);
int barrier_wait(int barrier_id);

void function();
#endif //USERLEVELTHREADS_LIBRARY_H
