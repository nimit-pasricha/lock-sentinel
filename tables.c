#include "tables.h"

#include <stdlib.h>

#define TABLE_SIZE 1024

// -------- Lock owned by Thread HashTable --------

typedef struct lock_node {
  pthread_mutex_t* lock_addr;  // Key
  pthread_t owner_thread;      // Value
  struct lock_node* next;      // Next node in collision chain
} lock_node_t;

static lock_node_t* lock_table[TABLE_SIZE];

static unsigned int hash_ptr(void* ptr) {
  return ((unsigned long)ptr >> 6) % TABLE_SIZE;
}

void register_lock_owner(pthread_mutex_t* mutex, pthread_t thread_id) {
  unsigned int index = hash_ptr(mutex);

  lock_node_t* new_node = malloc(sizeof(lock_node_t));
  new_node->lock_addr = mutex;
  new_node->owner_thread = thread_id;

  new_node->next = lock_table[index];  // insert at head
  lock_table[index] = new_node;
}

void unregister_lock(pthread_mutex_t* mutex) {
  unsigned int index = hash_ptr(mutex);
  lock_node_t** current = &lock_table[index];

  // scan this bucket for the lock
  while (*current) {
    lock_node_t* entry = *current;
    if (entry->lock_addr == mutex) {
      *current = entry->next;
      free(entry);
      return;
    }
    current = &entry->next;
  }
}

// -------- Thread waiting for Lock HashTable --------

// Node for Thread->Waiting_for map
typedef struct wait_node {
  pthread_t thread;        // Key
  pthread_mutex_t* lock;   // Value
  struct wait_node* next;  // Next node in collision chain
} wait_node_t;

static wait_node_t* wait_table[TABLE_SIZE];

static unsigned int hash_tid(pthread_t tid) {
  return ((unsigned long)tid) % TABLE_SIZE;
}

void register_thread_waiting_lock(pthread_t thread, pthread_mutex_t* mutex) {
  unsigned int index = hash_tid(thread);

  wait_node_t* node = malloc(sizeof(wait_node_t));
  node->thread = thread;
  node->lock = mutex;

  node->next = wait_table[index]; // insert at head
  wait_table[index] = node;
}