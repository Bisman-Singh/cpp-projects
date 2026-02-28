# LRU Cache

A generic Least Recently Used (LRU) cache implementation in C++17 with O(1) get and put operations.

## Features

- Template class `LRUCache<K, V>` supporting any key/value types
- O(1) get, put, erase, and contains operations using a doubly-linked list + hash map
- Automatic eviction of least recently used entries when capacity is exceeded
- Cache hit/miss statistics and hit rate tracking
- Pretty-print cache state showing MRU to LRU order
- Benchmark comparing LRU cache vs unbounded `std::unordered_map`

## Build

```bash
make
```

## Usage

```bash
./lrucache
```

The program demonstrates:
1. Basic cache operations (insert, access, eviction, update, erase)
2. Template flexibility with different key/value types
3. Performance benchmark with Zipf-like access patterns

## Clean

```bash
make clean
```
