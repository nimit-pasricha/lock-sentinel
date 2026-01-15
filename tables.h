#ifndef TABLES_H
#define TABLES_H

#include <pthread.h>

void lock_graph();
void unlock_graph();

typedef struct lock_node {
  pthread_mutex_t* lock_addr;  // Key
  pthread_t owner_thread;      // Value
  struct lock_node* next;      // Next node in collision chain
} lock_node_t;

// Node for Thread->Waiting_for map
typedef struct wait_node {
  pthread_t thread;        // Key
  pthread_mutex_t* lock;   // Value
  struct wait_node* next;  // Next node in collision chain
} wait_node_t;

// Hashtable for what thread is holding what lock.
void register_lock_owner(pthread_mutex_t* mutex, pthread_t thread_id);
void unregister_lock_owner(pthread_mutex_t* mutex);
pthread_t get_lock_owner(pthread_mutex_t* mutex);

// Hashtable of what lock is being waited for by what thread
void register_thread_waiting_lock(pthread_t thread, pthread_mutex_t* mutex);
void unregister_thread_waiting_lock(pthread_t thread);
pthread_mutex_t* get_awaited_lock(pthread_t thread);

int contains_cycle(pthread_t curr_thread, pthread_t start_thread, int depth);

#endif