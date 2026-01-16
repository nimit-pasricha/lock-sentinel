/**
 * Simple test program that is guaranteed to deadlock
 */

#include <pthread.h>
#include <stdio.h>
#include <unistd.h> // For sleep()

pthread_mutex_t lock_A = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_B = PTHREAD_MUTEX_INITIALIZER;

void *worker_one(void *arg)
{
    printf("Thread 1: Trying to acquire lock A...\n");
    pthread_mutex_lock(&lock_A);
    printf("Thread 1: Acquired lock A...\n");

    // to force other thread to run to guarantee deadlock
    sleep(1);

    printf("Thread 1: Trying to acquire lock B...\n");
    pthread_mutex_lock(&lock_B);

    // None of this should run cuz above line should deadlock
    printf("Thread 1: Acquired lock B...\n");

    pthread_mutex_unlock(&lock_B);
    pthread_mutex_unlock(&lock_A);
    return NULL;
}

void *worker_two(void *arg)
{
    printf("Thread 2: Trying to acquire lock B...\n");
    pthread_mutex_lock(&lock_B);
    printf("Thread 2: Acquired lock B...\n");

    // to force other thread to run to guarantee deadlock
    sleep(1);

    printf("Thread 2: Trying to acquire lock A...\n");
    pthread_mutex_lock(&lock_A);
    printf("Thread 2: Acquired lock A...\n");

    pthread_mutex_unlock(&lock_B);
    pthread_mutex_unlock(&lock_A);
    return NULL;
}

int main()
{
    pthread_t t1, t2;
    pthread_create(&t1, NULL, worker_one, NULL);
    pthread_create(&t2, NULL, worker_two, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("Main: Deadlock did not occur.\n");
    return 0;
}
