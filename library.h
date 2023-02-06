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
} ult;

void start();
void stop();

void print_array();
void init_array();

void scheduler();

int ult_self(void);
void ult_init(long period);
int ult_create(int* created_id, void (*function)(void*), void* arg);
void ult_join(int thread_to_wait_for_id);
void ult_exit();

void function();
#endif //USERLEVELTHREADS_LIBRARY_H
