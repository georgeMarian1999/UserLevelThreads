#include "library.h"

// The context we use for the main function
ucontext_t maincontext;

ult main_thread;
ult *current_thread;
int exit_current_thread = 0;
struct itimerval timer;



// The variable we use to swap between threads in the threads array;
int current_thread_position = 0;
// The thread array;
ult** thread_array;


void start() {
    setitimer(ITIMER_VIRTUAL, &timer, 0);
}

void stop() {
    setitimer(ITIMER_VIRTUAL, 0, 0);
}

void print_array() {
    printf("\n [");
    for(int i = 1; i < 100; i++) {
        printf("%d,", thread_array[i]->id);
    }
    printf("]\n");
}

void init_array() {
    thread_array = (ult**) malloc(100 * sizeof(ult));
    for(int i = 0; i < 100; i++) {
        thread_array[i] = (ult*) malloc(sizeof(ult));
        thread_array[i]->status = -1;
    }
}

ult* get_thread(int position) {
    return thread_array[position];
}
int ult_equal(ult thread1, ult thread2) {
    return thread1.id == thread2.id;
}

void scheduler() {
    ult *previous, *next;
    stop();

    if (getcontext(&maincontext) == -1) {
        printf("Error getting the context");
        exit(EXIT_FAILURE);
    }

    previous = current_thread;

    if (!exit_current_thread) {
        current_thread->status = 0;
        print_array();
    }
    else {
        exit_current_thread = 0;
    }

    // Getting the next ready to use thread
    current_thread_position = (current_thread_position + 1) % 100;
    while (thread_array[current_thread_position]->status != 0) {
        current_thread_position = (current_thread_position + 1) % 100;
    }
    next = thread_array[current_thread_position];
    current_thread = next;

    start();
    if (swapcontext(&(previous->ucontext), &(next->ucontext)) == -1) {
        printf("Error while swapping context\n");
    }
}

ult ult_self() {
    return *current_thread;
}

void ult_exit() {
    current_thread->status = 1;
    print_array();
    exit_current_thread = 1;
    scheduler();
}

void* function () {
    for(int i = 0; i <= 10000; i++) {
        printf("Thread %d printed number %d\n", current_thread->id , i);
    }
    ult_exit();
    return NULL;
}


int ult_create(ult *thread, void* function) {
    thread_array[current_thread_position]->id = current_thread_position;

    if (getcontext(&(thread_array[current_thread_position]->ucontext)) == -1) {
       printf("Error while creating the thread %d\n", thread_array[current_thread_position]->id);
    }

    // New stack for the thread;
    thread_array[current_thread_position]->ucontext.uc_link = &maincontext;
    thread_array[current_thread_position]->ucontext.uc_stack.ss_sp = malloc(SIGSTKSZ);
    thread_array[current_thread_position]->ucontext.uc_stack.ss_size = SIGSTKSZ;

    thread_array[current_thread_position]->status = 0;
    // Creating the context for the current thread;
    makecontext(&(thread_array[current_thread_position]->ucontext), function , 2);
    current_thread_position ++;
    return 0;
}

void ult_init (long period) {

    init_array();
    // Setting the timers for the alarm.
    timer.it_value.tv_sec = period / 1000000;
    timer.it_value.tv_usec = period;
    timer.it_interval.tv_sec = period / 1000000;
    timer.it_interval.tv_usec = period;
    start();

    // Setting signal handler.
    signal(SIGVTALRM, scheduler);

    // Save the main context;
    main_thread.id = -1;
    if (getcontext(&(main_thread.ucontext)) == -1) {
        printf("Error while getting the main context\n");
        exit(EXIT_FAILURE);
    }

    current_thread = &main_thread;
}

int ult_join(ult thread) {

    // If I am the thread just return 0;
    if (ult_equal(thread, ult_self())) {
        return 0;
    }

    ult* crt_thread = NULL;

    while (1) {
        for (int i = 0; i < 100 ; i++) {
            crt_thread = get_thread(i);
            if (ult_equal(thread, *crt_thread)) {
                if (crt_thread->status == 1) {
                    return 1;
                }
            }
        }
        ult_yield();
    }
}

int ult_yield() {
    stop();
    start();
    scheduler();
    return 0;
}



int main(void) {
    printf("Hello, World!\n");

    ult_init(10);

    for(int i = 0; i < 100; i++) {
        ult_create(thread_array[i], function());
    }
    current_thread_position = 0;
    for(int i = 0; i < 100; i++) {
        ult_join(*thread_array[i]);
    }

    return 0;
}
