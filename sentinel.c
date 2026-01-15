#define _GNU_SOURCE // for RTLD_NEXT

#include <stdio.h>
#include <dlfcn.h> // For dlsym() to find the real functions
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#define LOCK_TABLE_SIZE 1024



// RESOURCE ALLOCATION GRAPH OF LOCKS

typedef struct lock_node
{
    pthread_mutex_t *lock_addr; // Key
    pthread_t owner_thread;     // Value
    struct lock_node *next;     // Next node in LinkedList
 } lock_node_t;

static lock_node_t *lock_table[LOCK_TABLE_SIZE];



// LOCK AND UNLOCK WRAPPERS

typedef int (*pthread_mutex_lock_t)(pthread_mutex_t *);
typedef int (*pthread_mutex_unlock_t)(pthread_mutex_t *);

static pthread_mutex_lock_t real_lock_fn = NULL;
static pthread_mutex_unlock_t real_unlock_fn = NULL;

__attribute__((constructor)) void init_guard()
{
    // dlsym finds address of requested function
    // RTLD_NEXT to skip the one in this file and find the next one in library order.
    real_lock_fn = (pthread_mutex_lock_t)dlsym(RTLD_NEXT, "pthread_mutex_lock");
    real_unlock_fn = (pthread_mutex_lock_t)dlsym(RTLD_NEXT, "pthread_mutex_unlock");

    if (!real_lock_fn || !real_unlock_fn)
    {
        fprintf(stderr, "[ERROR] init_guard: Failed to find real pthread functions.\n");
    }
    else
    {
        fprintf(stderr, "[INFO] init_guard: Library loaded successfully.\n");
    }
}

// TODO: write wrapers
int pthread_mutex_lock(pthread_mutex_t *mutex)
{
    // stderr because unbuffered
    fprintf(stderr, "[INFO] Thread %lu requesting lock %p\n",
            (unsigned long)pthread_self(), (void *)mutex);

    int result = real_lock_fn(mutex);

    fprintf(stderr, "[INFO] Thread %lu acquired lock %p\n",
            (unsigned long int)pthread_self(), (void *)mutex);

    return result;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
    fprintf(stderr, "[INFO] Thread %lu releasing lock %p\n",
            (unsigned long int)pthread_self(), (void *)mutex);

    return real_unlock_fn(mutex);
}