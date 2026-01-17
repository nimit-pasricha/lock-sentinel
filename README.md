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

## Build

**Dependencies:** Requires `gcc`, `make`, `graphviz`.

Note that `graphviz` is needed to generate a PNG of the Resource Allocation Graph. You do not need this dependency if you don't care for the graph. Everything should work fine without it, but you might see some warnings in the output.

**Compilation:** Simply run `make libsentinel.so` in the project directory

This produces `libsentinel.so`, the shared library to be preloaded when running your executable.

## Usage

You do not need to link your program against the sentinel. Just inject it at runtime using the `LD_PRELOAD` environment variable.

```bash
LD_PRELOAD=./libsentinel.so ./your_executable
```

To see a quick demo, you can use the given `deadlock.c` file.

```bash
make # in the project directory

LD_PRELOAD=./libsentinel.so ./deadlock
```

## Configuration

The sentinel looks for a `sentinel.ini` file in the current directory.

Example `sentinel.ini`:
```Ini
; Default: log the resource allocation graph and let deadlock occur
policy=freeze
; Cancel the lock that is about to cause deadlock and return EDEADLK
;policy=return
; Only cancel a lock if a younger thread is waiting for an older one and return EDEADLK
;policy=wait_die

; Absolute or Relative path to resource allocation graph image.
graph_file_path=./sample.dot
; Default: Leave empty if don't want graph
;graph_file_path=
```

## Cons and Future Work

* Ideally, I hoped to implement a self-healing system to prevent deadlocks by rolling back locks made by some threads and putting them to sleep to allow others threads to continue until it is safe for the original thread to continue running without deadlock. I think this might be impossible using an external library as it would involve stopping threads in the middle of their critical section, without undoing the things they've already done in that critical section.

* Only supports mutex locks, which is still helpful if you have a bunch of locks, but pretty useless if you're just using one.

* Performance: Acquiring a lock is already fairly expensive, and this adds the overhead of running DFS through a graph. The DFS itself isn't particularly slow. The main issue is that all threads have to acquire a single lock to even access the resource graph. A way to improve this might be to dynamically create locks for different sections of the graph as it grows, and only have threads acquire a lock for their specific area, though this would add additional overhead of figuring out which section a thread is trying to access.