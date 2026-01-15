#ifndef TABLES_H
#define TABLES_H

#include <pthread.h>

// Initialize both hash tables (call this once)
void init_tables();

// --- LOCK REGISTRY (Who owns this lock?) ---
// Maps: pthread_mutex_t* -> pthread_t
void table_map_lock(pthread_mutex_t *lock, pthread_t owner);
void table_unmap_lock(pthread_mutex_t *lock);
pthread_t table_get_owner(pthread_mutex_t *lock);

// --- WAIT REGISTRY (Who is this thread waiting for?) ---
// Maps: pthread_t -> pthread_mutex_t*
void table_map_wait(pthread_t thread, pthread_mutex_t *lock);
void table_unmap_wait(pthread_t thread);
pthread_mutex_t* table_get_wait(pthread_t thread);

#endif