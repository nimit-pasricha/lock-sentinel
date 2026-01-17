# Lock Sentinel

Lock-Sentinel is a userspace library that automatically detects, visualizes, and prevents deadlocks caused by mutex locks in C applications without requiring source code modifications or recompilation.

It works by replacing `pthread_mutex_lock` and `pthread_mutex_unlock` calls in your source code with the library's respective wrappers at runtime.

## Features
* **Zero Source Code Modification Needed:** Uses `LD_PRELOAD` to attach to existing binaries. No recompilation needed.
* **Resource Allocation Graph:** Maintains a thread-safe, directed graph of all locks/threads (Nodes) and ownership/wait states (Edges) to detect deadlock.
* **Configurable Safety Policies:**
    * **Return:** Cancel the lock, and immediately return `EDEADLK` to the offender.
    * **Freeze:** Let the deadlock happen, generate a png of the graph to help debug.
    * **Wait-Die:** Inspired by DBMS, cancel the lock and return `EDEADLK` only if younger thread is waiting for an older thread. We are able to avoid deadlock when all threads follow this heuristic.
* **Visual Diagnostics:** Automatically generates **PNG graphs** of the deadlock state (using Graphviz), showing exactly which threads and locks caused the cycle.