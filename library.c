#include "library.h"


struct itimerval timer;
// The variable we use to swap between threads in the threads array;
int current_thread_id = 0;
// The thread array;
ult thread_array[102];


void start() {
    setitimer(ITIMER_VIRTUAL, &timer, NULL);
}

void stop() {
    setitimer(ITIMER_VIRTUAL, 0, 0);
}

void print_array() {
    printf("\n [");
    for(int i = 1; i < 100; i++) {
        printf(" Thread %d with status %d\n", thread_array[i].id, thread_array[i].status);
    }
    printf("]\n");
}

void init_array() {
    for(int i = 0; i < 100; i++) {
        thread_array[i].id = i;
        thread_array[i].status = -1;
        thread_array[i].waitsFor = -2;
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
            searching = 0;
        }
        // Else we go to the next thread
        else {
            next_thread_id= (next_thread_id+ 1) % 100;
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
     // for (int j = 0 ; j <= 40; j++) {
        f();
        for(int i = 0; i <= 1000; i++) {
            printf("Thread %d printed number %d\n", thread_array[current_thread_id].id , i);
        }
    // }
}


int ult_create(int* created_id, void function(void*), void* arg) {
    int new_thread_id = -1;
    for (int i = 0; i < 100 ; i++) {
        if (thread_array[i].status == -1) {
            new_thread_id = i;
            break;
        }
    }
    thread_array[new_thread_id].status = 0;
    if (getcontext(&(thread_array[new_thread_id].ctx)) == -1) {
       printf("Error while creating the thread %d\n", thread_array[new_thread_id].id);
    }

    // New stack for the thread;
    thread_array[new_thread_id].ctx.uc_stack.ss_sp = malloc(SIGSTKSZ);
    thread_array[new_thread_id].ctx.uc_stack.ss_size = SIGSTKSZ;

    if (getcontext(&(thread_array[new_thread_id].end_ctx)) == -1) {
        printf("Error while creating the thread %d\n", thread_array[new_thread_id].id);
    }
    thread_array[new_thread_id].end_ctx.uc_stack.ss_sp = malloc(SIGSTKSZ);
    thread_array[new_thread_id].end_ctx.uc_stack.ss_size = SIGSTKSZ;
    makecontext(&(thread_array[new_thread_id].end_ctx), ult_exit, 0);
    thread_array[new_thread_id].ctx.uc_link = &thread_array[new_thread_id].end_ctx;

    // Creating the context for the current thread;
    makecontext(&(thread_array[new_thread_id].ctx), (void (*)(void)) function, 1, arg);
    *created_id = new_thread_id;
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

int ult_yield() {
    stop();
    start();
    scheduler();
    return 0;
}



int main(void) {
    printf("Hello, World!\n");
    int threads[50];
    ult_init(10);

    for(int i = 0; i < 2; i++) {
        ult_create(&threads[i], function, NULL);
    }
    for(int i = 0; i < 2; i++) {
        ult_join(threads[i]);
    }

    return 0;
}
