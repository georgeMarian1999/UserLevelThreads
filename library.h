#ifndef USERLEVELTHREADS_LIBRARY_H
#define USERLEVELTHREADS_LIBRARY_H


#include <stdio.h>
#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <malloc.h>
#include <signal.h>


typedef struct {
    int id;
    ucontext_t ucontext;
    int status; // -1 - not created; 0 - ready; 1 - finished;
    int waitsFor; // Do I need this?;
} ult;

void start();
void stop();

void print_array();
void init_array();
ult* get_thread(int position);

void scheduler();

ult ult_self(void);
int ult_equal(ult thread1, ult thread2);
void ult_init(long period);
int ult_create(ult *thread, void* function);
int ult_join(ult thread);
int ult_yield();
void ult_exit();





void* function();
void hello(void);
#endif //USERLEVELTHREADS_LIBRARY_H
