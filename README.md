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

### New solution paths in this batch

| Problem | Language | Source |
|---|---|---|
| Array left rotation | C | [c/array_left_rotation.c](c/array_left_rotation.c) |
| Longest increasing subsequence | Python | [python/longest_increasing_subsequence.py](python/longest_increasing_subsequence.py) |
| Dijkstra shortest paths | Rust | [rust/dijkstra.rs](rust/dijkstra.rs) |
| Topological sort | JavaScript | [javascript/topological_sort.js](javascript/topological_sort.js) |
| Trie | TypeScript | [typescript/trie.ts](typescript/trie.ts) |
| Merge intervals | C++ | [cpp/merge_intervals.cpp](cpp/merge_intervals.cpp) |
| Number of islands | Java | [java/NumberOfIslands.java](java/NumberOfIslands.java) |

## Layout and conventions

The original demonstrations remain at the repository root; additions are grouped in directories by implementation language (`c/`, `cpp/`, `java/`, `javascript/`, `python/`, `rust/`, and `typescript/`). Solutions are standalone examples rather than a single linked application.

Complexity notes should state time and auxiliary-space bounds, identifying relevant input sizes and assumptions where those affect the bound. Keep these notes near the algorithm or in its solution entry; do not imply a bound that the implementation does not meet.

## Tests

Run `scripts/test_all.sh` from any working directory to compile and run the repository's standalone demos and self-tests. Compiled artifacts go into a temporary directory that is removed on exit. C and C++ compilation uses strict warning flags. Java checks enable assertions and use strict `javac` lint flags when `javac` is available, otherwise they use the Java source-file launcher. TypeScript is type-checked in strict mode; Rust sources are compiled with `rustc --test` and warnings denied. Missing toolchains are reported as skipped, while actual test failures cause a non-zero exit status. Toolchain availability may vary by machine.
