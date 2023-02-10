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

    char* buffer; // NULL if no message to write or message got.
    int length_of_buffer; // Length of what you want to read or write
    int wait_for_channel; // -1  not waiting for any channel;  else id of the channel;
    int channel_wait_type; // 0 for read ; 1 for write
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


typedef struct {
    int id; //
    char buffer[100]; // the message
    int length; // length of the message;  if not created then is -1
    int capacity;
} ult_channel;


typedef struct {
    int thread_id;
    int size;// -1 if not registered for a read // number of bytes to read ;
} ult_reads;

typedef struct {
    int thread_id;
    int size;// -1 if not registered for a write // number of bytes to write ;
} ult_writes;

void start();
void stop();

void print_array();
void init_array();

void scheduler();

int ult_self(void);
void ult_init(long period);
int ult_create(int created_id, void (*function)(void*), void* arg);
void ult_join(int thread_to_wait_for_id);
char* ult_buffer();
void ult_exit();


void mutex_init(int mutex_id);
int mutex_lock(int mutex_id);
int mutex_unlock(int mutex_id);

void barrier_create(int id);
void barrier_init(int barrier_id, int capacity);
int barrier_wait(int barrier_id);

void channel_init(int length);
void channel_write(char* buffer, int length);
void channel_read(int length);
void channel_end_read();
void channel_end_write();
void copy_and_remove(char* source, char* destination, int length);


void function();
#endif //USERLEVELTHREADS_LIBRARY_H
