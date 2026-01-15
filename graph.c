#include "graph.h"

#include <stdlib.h>
#include <string.h>  // For memset

#define TABLE_SIZE 1024
#define MAX_DEPTH 64

// -------- Lock owned by Thread HashTable --------

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

void unregister_lock_owner(pthread_mutex_t* mutex) {
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

pthread_t get_lock_owner(pthread_mutex_t* mutex) {
  unsigned int index = hash_ptr(mutex);
  lock_node_t* curr = lock_table[index];
  while (curr) {
    if (curr->lock_addr == mutex) {
      return curr->owner_thread;
    }
    curr = curr->next;
  }
  return 0;  // Not found
}

// -------- Thread waiting for Lock HashTable --------

static wait_node_t* wait_table[TABLE_SIZE];

static unsigned int hash_tid(pthread_t tid) {
  return ((unsigned long)tid) % TABLE_SIZE;
}

void register_thread_waiting_lock(pthread_t thread, pthread_mutex_t* mutex) {
  unsigned int idx = hash_tid(thread);
  wait_node_t* node = malloc(sizeof(wait_node_t));
  node->thread = thread;
  node->lock = mutex;
  node->next = wait_table[idx];
  wait_table[idx] = node;
}

void unregister_thread_waiting_lock(pthread_t thread) {
  unsigned int idx = hash_tid(thread);
  wait_node_t** curr = &wait_table[idx];
  while (*curr) {
    wait_node_t* entry = *curr;
    if (pthread_equal(entry->thread, thread)) {
      *curr = entry->next;
      free(entry);
      return;
    }
    curr = &entry->next;
  }
}

pthread_mutex_t* get_awaited_lock(pthread_t thread) {
  unsigned int index = hash_tid(thread);
  wait_node_t* curr = wait_table[index];
  while (curr) {
    if (pthread_equal(curr->thread, thread)) {
      return curr->lock;
    }
    curr = curr->next;
  }
  return NULL;  // Not waiting
}

// -------- Check for cycle in graph --------

/**
 * @return 0 if doesn't contain cycle, 1 if contains cycle
 */
int contains_cycle(pthread_t curr_thread, pthread_t start_thread, int depth) {
  if (depth > MAX_DEPTH) {
    return 0;  // Graph too deep, give up, assume no cycle
  }

  pthread_mutex_t* wanted_lock = get_awaited_lock(curr_thread);
  if (wanted_lock == NULL) {
    return 0;
  }

  pthread_t owner = get_lock_owner(wanted_lock);
  if (owner == 0) {
    return 0;
  }

  if (pthread_equal(owner, start_thread)) {
    return 1;
  }

  return contains_cycle(owner, start_thread, depth + 1);
}

// -------- MAIN --------

// This will probably give us balls performance. Maybe try reader-writer
static pthread_mutex_t graph_lock = PTHREAD_MUTEX_INITIALIZER;
static int (*real_lock)(pthread_mutex_t*) = NULL;
static int (*real_unlock)(pthread_mutex_t*) = NULL;

void init_tables(int (*real_lock_fn)(pthread_mutex_t*),
                 int (*real_unlock_fn)(pthread_mutex_t*)) {
  real_lock = real_lock_fn;
  real_unlock = real_unlock_fn;
  memset(lock_table, 0, sizeof(lock_table));
  memset(wait_table, 0, sizeof(wait_table));
}

void lock_graph() { real_lock(&graph_lock); }

void unlock_graph() { real_unlock(&graph_lock); }