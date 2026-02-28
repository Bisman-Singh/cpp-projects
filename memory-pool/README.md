# Memory Pool Allocator

A custom fixed-size memory pool allocator with free list recycling, including benchmarks comparing it against standard `new`/`delete`.

## Features

- Pre-allocates a contiguous memory block for fixed-size chunks
- O(1) allocation and deallocation via free list
- Automatic chunk recycling
- Alignment-aware (respects `max_align_t`)
- Correctness test suite
- Performance benchmarks at multiple scales (1K to 1M allocations)

## Build

```bash
make
```

## Usage

```bash
./mempool
```

Runs correctness tests followed by benchmarks comparing pool allocation vs standard `new`/`delete` for 1,000 to 1,000,000 objects.

## Clean

```bash
make clean
```
