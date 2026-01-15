#define _GNU_SOURCE  // for RTLD_NEXT

#include <dlfcn.h>  // For dlsym() to find the real functions
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define LOCK_TABLE_SIZE 1024

// -------- Resource Allocation Graph of lcoks --------

// TODO: this will probably obliterate performance. Replace with 1 lock per
// bucket.
static pthread_mutex_t sentinel_global_lock = PTHREAD_MUTEX_INITIALIZER;

// -------- Lock and Unlock Wrappers --------

typedef int (*pthread_mutex_lock_t)(pthread_mutex_t*);
typedef int (*pthread_mutex_unlock_t)(pthread_mutex_t*);

static pthread_mutex_lock_t real_lock_fn = NULL;
static pthread_mutex_unlock_t real_unlock_fn = NULL;

__attribute__((constructor)) void init_guard() {
  // dlsym finds address of requested function
  // RTLD_NEXT to skip the one in this file and find the next one in library
  // order.
  real_lock_fn = (pthread_mutex_lock_t)dlsym(RTLD_NEXT, "pthread_mutex_lock");
  real_unlock_fn =
      (pthread_mutex_lock_t)dlsym(RTLD_NEXT, "pthread_mutex_unlock");

  if (!real_lock_fn || !real_unlock_fn) {
    fprintf(stderr,
            "[ERROR] init_guard: Failed to find real pthread functions.\n");
  }
}

int pthread_mutex_lock(pthread_mutex_t* mutex) {
  pthread_t curr_thread_id = pthread_self();

  int result = real_lock_fn(mutex);
  if (result == 0) {
    // NOTE: make sure you use the real_ locks otherwise we infinite loop
    real_lock_fn(&sentinel_global_lock);
    register_lock(mutex, curr_thread_id);
    real_unlock_fn(&sentinel_global_lock);
  }

  return result;
}

int pthread_mutex_unlock(pthread_mutex_t* mutex) {
  real_lock_fn(&sentinel_global_lock);
  unregister_lock(mutex);
  real_unlock_fn(&sentinel_global_lock);
  return real_unlock_fn(mutex);
}