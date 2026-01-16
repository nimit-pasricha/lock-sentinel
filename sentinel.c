#define GNU_SOURCE // for RTLD_NEXT

#include <dlfcn.h> // For dlsym() to find the real functions
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "graph.h"
#include "config.h"

#define LOCK_TABLE_SIZE 1024

typedef int (*pthread_mutex_lock_t)(pthread_mutex_t *);

typedef int (*pthread_mutex_unlock_t)(pthread_mutex_t *);

static pthread_mutex_lock_t real_lock = nullptr;
static pthread_mutex_unlock_t real_unlock_fn = nullptr;

__attribute__((constructor)) void init_guard() {
    // dlsym finds address of requested function. RTLD_NEXT to skip the one in
    // this file and find the next one in library order.
    real_lock = (pthread_mutex_lock_t) dlsym(RTLD_NEXT, "pthread_mutex_lock");
    real_unlock_fn =
            (pthread_mutex_lock_t) dlsym(RTLD_NEXT, "pthread_mutex_unlock");

    if (!real_lock || !real_unlock_fn) {
        fprintf(stderr,
                "[ERROR] init_guard: Failed to find real pthread functions.\n");
    }

    init_tables(real_lock, real_unlock_fn);
    load_config();
}

int pthread_mutex_lock(pthread_mutex_t *mutex) {
    pthread_t self = pthread_self();

    lock_graph();
    pthread_t existing_owner = get_lock_owner(mutex);

    if (existing_owner != 0) {
        if (pthread_equal(existing_owner, self)) {
            fprintf(stderr, "[ERROR] pthread_mutex_lock: Recursive locking on %p\n",
                    (void *) mutex);
            unlock_graph();
            return EDEADLK;
        }

        if (contains_cycle(existing_owner, self, 0) == 1) {
            // TODO: Log the cycle for debugging, and do different thing than just
            // deny lock
            fprintf(stderr, "[INFO] DEADLOCK PREVENTED: Thread %lu -> Lock %p\n",
                    (unsigned long) self, (void *) mutex);

            unlock_graph();
            return EDEADLK;
        }
    }

    register_thread_waiting_lock(self, mutex);
    unlock_graph();

    int result = real_lock(mutex);

    lock_graph();
    unregister_thread_waiting_lock(self);
    if (result == 0) {
        register_lock_owner(mutex, self);
    }

    unlock_graph();

    return result;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex) {
    lock_graph();
    unregister_lock_owner(mutex);
    unlock_graph();
    return real_unlock_fn(mutex);
}
