#define _GNU_SOURCE   // for RTLD_NEXT

#include <stdio.h>
#include <dlfcn.h>    // For dlsym() to find the real functions
#include <pthread.h> 
#include <unistd.h>  

typedef int (*pthread_mutex_lock_t) (pthread_mutex_t *);
typedef int (*pthread_mutex_unlock_t) (pthread_mutex_t *);

static pthread_mutex_lock_t real_lock_fn = NULL;
static pthread_mutex_unlock_t real_unlock_fn = NULL;