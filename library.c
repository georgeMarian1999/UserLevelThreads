#include "library.h"


struct itimerval timer;
struct itimerval timer_deadlock;

// The variable we use to swap between threads in the threads array;
int current_thread_id = 0;
// The thread array;
ult thread_array[102];
mutex mutex_array[100];
barrier barriers[20];
ult_channel channel;

int mutex_id_1;
int mutex_id_2;

void start_deadlock() {
    setitimer(ITIMER_REAL, &timer_deadlock, NULL);
}
void stop_deadlock() {
    setitimer(ITIMER_REAL, 0, 0);
}

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
        thread_array[i].at_barrier = -1;
        thread_array[i].wait_for_channel = -1;
        thread_array[i].channel_wait_type = -1;
    }
    for(int i = 0; i < 20; i++) {
        barriers[i].capacity = -1;
    }
    channel.length = -1;
}

void deadlock_detector() {
    int number_of_threads = 0;
    for (int i = 0; i < 100; i++) {
        if(thread_array[i].status == 0) {
            number_of_threads ++;
        }
    }
    for(int i = 0; i < 20; i ++){
        if(barriers[i].capacity > number_of_threads) {
            // Destroy the barrier
            barriers[i].capacity = -1;
        }
    }



        int wants_to_lock_mutex = thread_array[current_thread_id].wants_mutex_lock_id;
        int mutex_locked_by_thread = -1;
        for (int j = 0; j < 20 && mutex_locked_by_thread == -1; j++) {
            if (mutex_array[j].locked_by == current_thread_id) {
                mutex_locked_by_thread = mutex_array[j].id;
            }
        }
        printf("Current thread %d has lock %d\n", current_thread_id, mutex_locked_by_thread);
        // Detect if there is a deadlock for the mutex
        if (wants_to_lock_mutex != -1 && mutex_locked_by_thread != -1) {
            int mutex_locked_by = mutex_array[wants_to_lock_mutex].locked_by;
            if (mutex_locked_by != -1 && thread_array[mutex_locked_by].status == 0) {
                if(thread_array[mutex_locked_by].wants_mutex_lock_id == mutex_locked_by_thread) {
                    // Kill that thread
                    printf("Mutex deadlock found for threads %d ,%d\n", current_thread_id, mutex_locked_by);
                    printf("Current thread %d holding lock on %d\n", current_thread_id, mutex_locked_by_thread);
                    printf("Current thread is %d wanting to lock mutex %d\n", current_thread_id, thread_array[current_thread_id].wants_mutex_lock_id);
                    printf("Thread holding mutex %d is %d\n", mutex_array[wants_to_lock_mutex].id, mutex_locked_by);
                    mutex_array[wants_to_lock_mutex].locked_by = -1;
                    thread_array[mutex_locked_by].status = 1;
                    thread_array[mutex_locked_by].waitsFor = -2;
                    thread_array[mutex_locked_by].wants_mutex_lock_id = -1;
                    thread_array[mutex_locked_by].wants_mutex_unlock_id = -1;
                    thread_array[mutex_locked_by].at_barrier = -1;
                    thread_array[mutex_locked_by].wait_for_channel = -1;
                    thread_array[mutex_locked_by].channel_wait_type = -1;
                }
            }
        }
}

void scheduler() {
    int previous_thread_id, next_thread_id;
    stop();
    deadlock_detector();
    // stop_deadlock();

    // Getting the next ready to use thread
    next_thread_id = (current_thread_id + 1) % 100;
    int searching = 1;
    // We loop through all the threads to find the next one.
    while (searching == 1) {
        // If the thread is ready and not finished we continue
        if(thread_array[next_thread_id].status == 0) {

            // Decide wait for
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

            // Decide mutex lock
            int mutex_wants_to_lock = thread_array[next_thread_id].wants_mutex_lock_id;
            // If the next thread to be selected wants to lock a mutex
            if (mutex_wants_to_lock != -1) {

                // If the mutex we want to lock is not locked by anyone, or is already locked by me
                if (mutex_array[mutex_wants_to_lock].locked_by == -1) {
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


            // Decide mutex unlock
            int mutex_wants_to_unlock = thread_array[next_thread_id].wants_mutex_unlock_id;
            // If the next thread to be selected wants to unlock a mutex
            if (mutex_wants_to_unlock != -1) {
                // We unlock the mutex;
                mutex_array[mutex_wants_to_unlock].locked_by = -1;
                // We specify that the next thread doesn't want to unlock any mutex;
                thread_array[next_thread_id].wants_mutex_unlock_id = -1;
            }


            // If the next thread is waiting at the barrier
            if (thread_array[next_thread_id].at_barrier != -1 && barriers[thread_array[next_thread_id].at_barrier].capacity != -1) {
                // Decide waiting at barrier
                int threads_at_barrier = 1;
                // Save the barrier id;
                int barrier = thread_array[next_thread_id].at_barrier;

                // Verify how many are already waiting at the barrier
                for (int i = 0; i < 100; i++) {
                    if (thread_array[i].at_barrier == barrier && thread_array[i].status == 0 && i != next_thread_id) {
                        threads_at_barrier++;
                    }
                    if (threads_at_barrier == barriers[barrier].capacity) {
                        break;
                    }
                }

                // If all the threads that wait at the barrier reach the barrier capacity then break the barrier
                if (threads_at_barrier == barriers[barrier].capacity) {
                    threads_at_barrier = 1;
                    // We set for the next thread that he has no barrier
                    thread_array[next_thread_id].at_barrier = -1;
                    for (int i = 0; i < 100; i++) {
                        if (thread_array[i].at_barrier == barrier && thread_array[i].status == 0 && i != next_thread_id) {
                            threads_at_barrier++;
                            thread_array[i].at_barrier = -1;
                        }
                        if (threads_at_barrier == barriers[barrier].capacity) {
                            // Destroy the barrier if the capacity is reached
                            barriers[barrier].capacity = -1;
                            break;
                        }
                    }
                }
                else {
                    // If we can t break the barrier yet verify the next thread;
                    next_thread_id = (next_thread_id + 1) % 100;
                    continue;
                }

            }


            // Decide channel writing and reading
            // If the next thread is waiting for a channel to read or write
            if(thread_array[next_thread_id].wait_for_channel != -1) {

                // If the next thread wants to read from a channel
                if (thread_array[next_thread_id].channel_wait_type == 0) {
                    // If the length of what was written is greater then the
                    if (strlen(channel.buffer) > strlen(thread_array[next_thread_id].buffer)) {


                    }
                    else {
                        next_thread_id = (next_thread_id + 1) % 100;
                        continue;
                    }

                }

                // If the thread wants to write to the channel.
                if (thread_array[next_thread_id].channel_wait_type == 1) {
                    if (strlen(channel.buffer) > strlen(thread_array[next_thread_id].buffer)) {


                    }
                    else {
                        next_thread_id = (next_thread_id + 1) % 100;
                        continue;
                    }
                }
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
    // start_deadlock();
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
     for (int j = 0 ; j <= 20; j++) {
         f();
         printf("Thread %d waiting for lock %d\n", ult_self(), 3);
         mutex_lock(3);
         int wait = 0;
         for(long i = 0; i <= 1000000; i++) {
             wait++;
         }
         f();
         f();

         mutex_unlock(3);
         printf("Thread %d unlocking %d\n", ult_self(), 3);

         printf("Thread %d waiting for lock %d\n", ult_self(), 7);
         mutex_lock(7);
         for(int i = 0; i <= 10000; i++) {
             wait++;
         }
         f();
         f();

         mutex_unlock(7);
         printf("Thread %d unlocking %d\n", ult_self(), 7);
    }
}
void test_barrier() {
    printf("Thread %d is waiting at barrier\n", ult_self());
    for (int j = 0 ; j <= 400; j++) {
        f();
        int wait = 0;
        for(long i = 0; i <= 1000000; i++) {
            wait++;
        }
    }
    barrier_wait(8);
    printf("Thread %d after the barrier is down\n", ult_self());
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

    // signal(SIGALRM, deadlock_detector);
    timer_deadlock.it_value.tv_sec = 0;
    timer_deadlock.it_value.tv_usec = 900000;
    timer_deadlock.it_interval.tv_usec = 0;
    timer_deadlock.it_interval.tv_usec = 900000;

    // start_deadlock();
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

void barrier_init(int barrier_id, int capacity) {
    barriers[barrier_id].id = barrier_id;
    barriers[barrier_id].capacity = capacity;
}

int barrier_wait(int barrier_id) {
    if (barriers[barrier_id].capacity != -1) {
        thread_array[current_thread_id].at_barrier = barrier_id;
        scheduler();
    }
    return 1;
}

void channel_init(int length) {
    channel.id = 1;
    channel.length = length;
    channel.buffer = malloc(length * sizeof(char));
    mutex_init(channel.id);
}

void channel_write(char* buffer, int size) {
    if (channel.length != -1) {
        thread_array[current_thread_id].wait_for_channel = channel.id;
        thread_array[current_thread_id].channel_wait_type = 1;
        strcpy(thread_array[current_thread_id].buffer, buffer);
        thread_array[current_thread_id].length_of_buffer = size;
        scheduler();
    }
}

void channel_read(int length) {
    if (channel.length != -1) {
        thread_array[current_thread_id].wait_for_channel = channel.id;
        thread_array[current_thread_id].channel_wait_type = 0;
        thread_array[current_thread_id].length_of_buffer = length;
        scheduler();
    }
}

void test_mutex() {
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
}

void test_barriers() {
    int threads[50];
    for (int i = 1; i < 10; i++) {
        threads[i] = i;
    }
    ult_init(10);
    barrier_init(8, 20);

    for(int i = 1; i < 10; i++) {
        ult_create(threads[i], test_barrier, NULL);
    }
    for(int i = 1; i < 10; i++) {
        ult_join(threads[i]);
    }
    barriers[8].capacity = -1;
}

void test_channel() {
    int threads[50];
    for (int i = 1; i < 10; i++) {
        threads[i] = i;
    }
    channel_init(20);
    for(int i = 1; i < 10; i++) {
        ult_create(threads[i], function, NULL);
    }
    for(int i = 1; i < 10; i++) {
        ult_join(threads[i]);
    }
}

void f1() {
    f();
    f();
    mutex_lock(2);
    f();

    f();
    mutex_lock(5);
    mutex_unlock(2);
    f();
    f();

    f();
    f();
    mutex_unlock(5);
}

void f2() {
    f();
    f();
    mutex_lock(5);
    f();
    f();
    mutex_lock(2);
    mutex_unlock(5);
    f();
    f();

    f();
    f();
    mutex_unlock(2);
}

void test_mutex_deadlock() {
    int mutex_id1 = 2;
    int mutex_id2 = 5;
    mutex_init(mutex_id1);
    mutex_init(mutex_id2);
    ult_init(10);

    ult_create(1,f1,NULL);
    ult_create(2, f2, NULL);

    ult_join(1);
    ult_join(2);
}

int main(void) {

    // test_barriers();
    //  test_mutex();
    test_mutex_deadlock();
    return 0;
}
