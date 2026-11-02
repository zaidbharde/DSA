# Data Structures and Algorithms

A compact, multi-language index of standalone data-structure and algorithm demonstrations. Each source is intended to be readable and independently runnable.

## Problem index

### Existing root demonstrations

| Problem | Language | Source |
|---|---|---|
| Breadth-first search | C++ | [BFS.cpp](BFS.cpp) |
| Binary search | C++ | [BS.cpp](BS.cpp) |
| Singly linked list | C++ | [LL.cpp](LL.cpp) |
| Merge sort | C++ | [MS.cpp](MS.cpp) |
| Stack | C++ | [STACK.cpp](STACK.cpp) |
| Breadth-first search | Java | [BFSGraph.java](BFSGraph.java) |
| Binary search | Java | [BinarySearch.java](BinarySearch.java) |
| Singly linked list | Java | [LinkedList.java](LinkedList.java) |
| Merge sort | Java | [MergeSort.java](MergeSort.java) |
| Stack | Java | [Stack.java](Stack.java) |

### Previously added solutions (2026-10-31)

| Problem | Language | Source |
|---|---|---|
| Array left rotation | C | [c/array_left_rotation.c](c/array_left_rotation.c) |
| Longest increasing subsequence | Python | [python/longest_increasing_subsequence.py](python/longest_increasing_subsequence.py) |
| Dijkstra shortest paths | Rust | [rust/dijkstra.rs](rust/dijkstra.rs) |
| Topological sort | JavaScript | [javascript/topological_sort.js](javascript/topological_sort.js) |
| Trie | TypeScript | [typescript/trie.ts](typescript/trie.ts) |
| Merge intervals | C++ | [cpp/merge_intervals.cpp](cpp/merge_intervals.cpp) |
| Number of islands | Java | [java/NumberOfIslands.java](java/NumberOfIslands.java) |

### Solutions added (2026-11-01)

| Problem | Language | Source |
|---|---|---|
| Circular queue | C | [c/circular_queue.c](c/circular_queue.c) |
| Heap sort | C++ | [cpp/heap_sort.cpp](cpp/heap_sort.cpp) |
| Kruskal minimum spanning tree | Java | [java/KruskalMST.java](java/KruskalMST.java) |
| Coin change | Python | [python/coin_change.py](python/coin_change.py) |
| Disjoint set (union-find) | Rust | [rust/disjoint_set.rs](rust/disjoint_set.rs) |
| Sliding window maximum | JavaScript | [javascript/sliding_window_max.js](javascript/sliding_window_max.js) |
| Segment tree | TypeScript | [typescript/segment_tree.ts](typescript/segment_tree.ts) |

### Solutions added (2026-11-02)

| Problem | Language | Source |
|---|---|---|
| Maximum subarray | C | [c/maximum_subarray.c](c/maximum_subarray.c) |
| Quickselect | C++ | [cpp/quickselect.cpp](cpp/quickselect.cpp) |
| AVL tree | Java | [java/AvlTree.java](java/AvlTree.java) |
| Word Break | Python | [python/word_break.py](python/word_break.py) |
| LRU cache | JavaScript | [javascript/lru_cache.js](javascript/lru_cache.js) |
| Edit distance | TypeScript | [typescript/edit_distance.ts](typescript/edit_distance.ts) |

### Additional-language implementations

| Existing problem | Additional language | Source |
|---|---|---|
| Topological sort (also implemented in JavaScript above) | Rust | [rust/topological_sort.rs](rust/topological_sort.rs) |

## Recent improvements

The C++ binary search in [BS.cpp](BS.cpp) is safe on an empty vector and returns the first occurrence when a target is duplicated. The Java BFS in [BFSGraph.java](BFSGraph.java) uses `ArrayDeque` as its FIFO queue. The C++ merge-interval implementation sorts its by-value input and compacts merged intervals in that same vector before resizing, avoiding a separate result allocation while retaining closed-interval semantics and invalid-range validation. JavaScript Kahn traversal preallocates its FIFO worklist to the vertex count and advances a head index, avoiding front-removal shifts and queue resizing while preserving deterministic order and O(V + E) time.

## Layout and conventions

The original demonstrations remain at the repository root; additions are grouped in directories by implementation language (`c/`, `cpp/`, `java/`, `javascript/`, `python/`, `rust/`, and `typescript/`). The problem index identifies each language-specific implementation, and parallel implementations of a problem are listed separately from new problem entries. Solutions are standalone examples rather than a single linked application.

Complexity notes should state time and auxiliary-space bounds, identify relevant input sizes (for example, n items or V vertices and E edges), and name assumptions that affect the bound. Distinguish auxiliary space from input/output storage and do not imply a bound that the implementation does not meet.

## Tests

Run `scripts/test_all.sh` from any working directory to compile and run the repository's standalone demos and self-tests. Compiled artifacts go into a temporary directory that is removed on exit. C and C++ compilation uses strict warning flags. The test runner defines `MAXIMUM_SUBARRAY_TESTING` when compiling `c/maximum_subarray.c` so its guarded self-test `main` is executed. Java checks enable assertions and use strict `javac` lint flags when `javac` is available, otherwise they use the Java source-file launcher. TypeScript is type-checked in strict mode; Rust sources are compiled with `rustc --edition=2021 --test -D warnings`. Missing toolchains are reported as skipped, while actual test failures cause a non-zero exit status. Toolchain availability may vary by machine.
