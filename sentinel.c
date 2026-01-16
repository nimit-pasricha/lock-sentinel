#define GNU_SOURCE // for RTLD_NEXT

#include <dlfcn.h> // For dlsym() to find the real functions
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "graph.h"
#include "config.h"

#define LOCK_TABLE_SIZE 1024

// TODO: don't clutter output. Replace all prints with file logs

typedef int (*pthread_mutex_lock_t)(pthread_mutex_t *);

typedef int (*pthread_mutex_unlock_t)(pthread_mutex_t *);

static pthread_mutex_lock_t real_lock = NULL;
static pthread_mutex_unlock_t real_unlock = NULL;

__attribute__((constructor)) void init_guard()
{
    // dlsym finds address of requested function. RTLD_NEXT to skip the one in
    // this file and find the next one in library order.
    real_lock = (pthread_mutex_lock_t)dlsym(RTLD_NEXT, "pthread_mutex_lock");
    real_unlock =
        (pthread_mutex_lock_t)dlsym(RTLD_NEXT, "pthread_mutex_unlock");

    if (!real_lock || !real_unlock)
    {
        fprintf(stderr,
                "[FATAL] init_guard: Failed to find real pthread functions.\n");
        exit(1);
    }

    init_tables(real_lock, real_unlock);
    load_config();
}

int pthread_mutex_lock(pthread_mutex_t *mutex)
{
    pthread_t self = pthread_self();

    lock_graph();

    while (1)
    {
        pthread_t existing_owner = get_lock_owner(mutex);
        if (existing_owner == 0)
        {
            break;
        }

        if (pthread_equal(existing_owner, self))
        {
            unlock_graph();
            return EDEADLK;
        }

        if (contains_cycle(existing_owner, self, 0) == 1)
        {
            // TODO: Log the cycle for debugging
            if (global_config.policy == WAIT_DIE)
            {
                // Heuristic: smaller thread_id = older = higher priority
                if ((unsigned long)self < (unsigned long)existing_owner)
                {
                    // Older so priority
                    fprintf(stderr, "[INFO] Deadlock. Thread %lu waiting for %lu to retreat...\n",
                            (unsigned long)self, (unsigned long)existing_owner);
                    wait_for_graph_change();
                    continue; // Check again
                }
                // Younger so die
                fprintf(stderr, "[INFO] Deadlock! Thread %lu retreating to break cycle.\n",
                        (unsigned long)self);
                unlock_graph();
                return EDEADLK;
            }

            if (global_config.policy == FREEZE)
            {
                fprintf(stderr, "[INFO] Allowing deadlock to occur.\n");
                unlock_graph();
                return real_lock(mutex);
            }

            // RETURN policy
            fprintf(stderr, "[INFO] Deadlock. Aborting to save...\n");
            unlock_graph();
            return EDEADLK;
        }

        // reaching here means there is no cycle
        break;
    }

    register_thread_waiting_lock(self, mutex);
    unlock_graph();

    int result = real_lock(mutex);

    lock_graph();
    unregister_thread_waiting_lock(self);
    if (result == 0)
    {
        register_lock_owner(mutex, self);
    }

    unlock_graph();

    return result;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
    lock_graph();
    unregister_lock_owner(mutex);
    signal_graph_change();
    unlock_graph();
    return real_unlock(mutex);
}
