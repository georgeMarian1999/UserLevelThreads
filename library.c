#include "library.h"


struct itimerval timer;
// The variable we use to swap between threads in the threads array;
int current_thread_id = 0;
// The thread array;
ult thread_array[102];
mutex mutex_array[100];

int mutex_id_1;
int mutex_id_2;


void start() {
    setitimer(ITIMER_VIRTUAL, &timer, NULL);
}

void stop() {
    setitimer(ITIMER_VIRTUAL, 0, 0);
}

void print_array() {
    printf("\n [");
    for(int i = 0; i < 100; i++) {
        printf(" Thread %d with status %d\n", thread_array[i].id, thread_array[i].waitsFor);
    }
    printf("]\n");
}

void init_array() {
    for(int i = 0; i < 100; i++) {
        thread_array[i].id = i;
        thread_array[i].status = -1;
        thread_array[i].waitsFor = -2;
        thread_array[i].wants_mutex_lock_id = -1;
        thread_array[i].wants_mutex_unlock_id = -1;
    }
}

void scheduler() {
    int previous_thread_id, next_thread_id;
    stop();

    // Getting the next ready to use thread
    next_thread_id = (current_thread_id + 1) % 100;
    int searching = 1;
    // We loop through all the threads to find the next one.
    while (searching == 1) {
        // If the thread is ready and not finished we continue
        if(thread_array[next_thread_id].status == 0) {
            // if the thread is waiting for some other thread
            int waits_for_thread_id = thread_array[next_thread_id].waitsFor;
            if (waits_for_thread_id != -2) {

                // If that thread our thread is waiting for is finished we mark it
                if (thread_array[waits_for_thread_id].status == 1) {
                    thread_array[next_thread_id].waitsFor = -2;
                }
                // else we search for the next thread to give the context
                else {
                    next_thread_id = (next_thread_id + 1) % 100;
                    // Go to the next step of the while loop.
                    continue;
                }
            }

            int mutex_wants_to_lock = thread_array[next_thread_id].wants_mutex_lock_id;
            // If the next thread to be selected wants to lock a mutex
            if (mutex_wants_to_lock != -1) {

                // If the mutex we want to lock is not locked by anyone
                if (mutex_array[mutex_wants_to_lock].locked_by == -1 ) {
                    printf("Thread %d got lock mutex %d\n", next_thread_id, mutex_wants_to_lock);
                    // We specify that the next thread to be selected doesn't want to lock anyone because he acquired the lock
                    thread_array[next_thread_id].wants_mutex_lock_id = -1;
                    // We lock the mutex for the next thread to be selected
                    mutex_array[mutex_wants_to_lock].locked_by = next_thread_id;
                }
                else {
                    // If the mutex we want to lock is already locked go to next thread.
                    next_thread_id = (next_thread_id + 1) % 100;
                    continue;
                }
            }

            int mutex_wants_to_unlock = thread_array[next_thread_id].wants_mutex_unlock_id;
            // If the next thread to be selected wants to unlock a mutex
            if (mutex_wants_to_unlock != -1) {
                // We unlock the mutex;
                mutex_array[mutex_wants_to_unlock].locked_by = -1;
                // We specify that the next thread doesn't want to unlock any mutex;
                thread_array[next_thread_id].wants_mutex_unlock_id = -1;
            }

            searching = 0;
        }
        // Else we go to the next thread
        else {
            next_thread_id= (next_thread_id + 1) % 100;
        }

    }
    previous_thread_id = current_thread_id;
    current_thread_id = next_thread_id;

    start();

    if (swapcontext(&(thread_array[previous_thread_id].ctx), &(thread_array[current_thread_id].ctx)) == -1) {
        printf("Error while swapping context\n");
    }
}

int ult_self() {
    return current_thread_id;
}

void ult_exit() {
    thread_array[current_thread_id].status = 1;
    scheduler();
}

void f() {
    int x = 0;
    for(long i = 0; i < 1000000; ++i) {
        x = x + 1;
    }
}

void function () {
     for (int j = 0 ; j <= 30; j++) {
         f();
         mutex_lock(3);
         printf("Thread %d waiting for lock %d\n", ult_self(), 3);
         int wait = 0;
         for(long i = 0; i <= 1000000; i++) {
             wait++;
         }
         f();
         printf("Thread %d unlocking %d\n", ult_self(), 3);
         mutex_unlock(3);

         mutex_lock(7);
         printf("Thread %d waiting for lock %d\n", ult_self(), 7);
         for(int i = 0; i <= 10000; i++) {
             wait++;
         }
         printf("Thread %d unlocking %d\n", ult_self(), 7);
         mutex_unlock(7);
    }
}


int ult_create(int created_id, void function(void*), void* arg) {
    thread_array[created_id].status = 0;
    if (getcontext(&(thread_array[created_id].ctx)) == -1) {
       printf("Error while creating the thread %d\n", thread_array[created_id].id);
    }

    // New stack for the thread;
    thread_array[created_id].ctx.uc_stack.ss_sp = malloc(SIGSTKSZ);
    thread_array[created_id].ctx.uc_stack.ss_size = SIGSTKSZ;

    if (getcontext(&(thread_array[created_id].end_ctx)) == -1) {
        printf("Error while creating the thread %d\n", thread_array[created_id].id);
    }
    thread_array[created_id].end_ctx.uc_stack.ss_sp = malloc(SIGSTKSZ);
    thread_array[created_id].end_ctx.uc_stack.ss_size = SIGSTKSZ;
    makecontext(&(thread_array[created_id].end_ctx), ult_exit, 0);
    thread_array[created_id].ctx.uc_link = &thread_array[created_id].end_ctx;

    // Creating the context for the current thread;
    makecontext(&(thread_array[created_id].ctx), (void (*)(void)) function, 1, arg);
    // *created_id = created_id;
    return 0;
}

void ult_init (long period) {
    init_array();
    // Setting the timers for the alarm.

    // Init the main context;
    if (getcontext(&(thread_array[0].ctx)) == -1) {
        printf("Error creating the main context\n");
    }
    thread_array[0].status = 0;
    thread_array[0].waitsFor = -2;
    thread_array[0].wants_mutex_lock_id = -1;
    thread_array[0].wants_mutex_unlock_id = -1;
    current_thread_id = 0;

    // Setting signal handler.
    signal(SIGVTALRM, scheduler);
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 1;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 1;
    start();
}

void ult_join(int thread_to_wait_for_id) {
    thread_array[current_thread_id].waitsFor = thread_to_wait_for_id;
    scheduler();
}

void mutex_init(int mutex_id) {
    mutex_array[mutex_id].id = mutex_id;
    mutex_array[mutex_id].status = 0;
    mutex_array[mutex_id].locked_by = -1;
}

int mutex_lock(int mutex_id) {
    // If the mutex is created mark the current thread with wants_mutex_lock_id to the lock that he wants to lock
    if (mutex_array[mutex_id].status == 0) {
        thread_array[current_thread_id].wants_mutex_lock_id = mutex_id;
        scheduler();
    }
    else return -1;
}

int mutex_unlock(int mutex_id) {
    // If the current mutex is created mark in the thread array the current thread that he wants to unlock this mutex
    if (mutex_array[mutex_id].status == 0) {
        thread_array[current_thread_id].wants_mutex_unlock_id = mutex_id;
        scheduler();
    }
    else return -1;
}


int main(void) {
    int threads[50];
    for (int i = 1; i < 10; i++) {
        threads[i] = i;
    }
    mutex_id_1 = 3;
    mutex_id_2 = 7;
    mutex_init(mutex_id_1);
    mutex_init(mutex_id_2);
    ult_init(10);

    for(int i = 1; i < 10; i++) {
        ult_create(threads[i], function, NULL);
    }
    for(int i = 1; i < 10; i++) {
        ult_join(threads[i]);
    }

    return 0;
}
