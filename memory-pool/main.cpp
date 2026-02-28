#include <iostream>
#include <vector>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cassert>
#include <iomanip>
#include <new>

class MemoryPool {
public:
    MemoryPool(size_t chunk_size, size_t chunk_count)
        : chunk_size_(align_up(chunk_size, alignof(std::max_align_t))),
          chunk_count_(chunk_count),
          pool_(nullptr),
          free_head_(nullptr),
          allocated_count_(0) {
        pool_ = static_cast<uint8_t*>(::operator new(chunk_size_ * chunk_count_));
        build_free_list();
    }

    ~MemoryPool() {
        ::operator delete(pool_);
    }

    MemoryPool(const MemoryPool&) = delete;
    MemoryPool& operator=(const MemoryPool&) = delete;

    void* allocate() {
        if (!free_head_) {
            throw std::bad_alloc();
        }
        FreeNode* node = free_head_;
        free_head_ = node->next;
        ++allocated_count_;
        return static_cast<void*>(node);
    }

    void deallocate(void* ptr) {
        if (!ptr) return;

#ifndef NDEBUG
        auto addr = reinterpret_cast<uintptr_t>(ptr);
        auto base = reinterpret_cast<uintptr_t>(pool_);
        assert(addr >= base && addr < base + chunk_size_ * chunk_count_);
        assert((addr - base) % chunk_size_ == 0);
#endif

        auto* node = static_cast<FreeNode*>(ptr);
        node->next = free_head_;
        free_head_ = node;
        --allocated_count_;
    }

    size_t chunk_size() const { return chunk_size_; }
    size_t chunk_count() const { return chunk_count_; }
    size_t allocated() const { return allocated_count_; }
    size_t available() const { return chunk_count_ - allocated_count_; }

private:
    struct FreeNode {
        FreeNode* next;
    };

    static size_t align_up(size_t size, size_t alignment) {
        size_t min_size = sizeof(FreeNode);
        if (size < min_size) size = min_size;
        return (size + alignment - 1) & ~(alignment - 1);
    }

    void build_free_list() {
        free_head_ = nullptr;
        for (size_t i = chunk_count_; i > 0; --i) {
            auto* node = reinterpret_cast<FreeNode*>(pool_ + (i - 1) * chunk_size_);
            node->next = free_head_;
            free_head_ = node;
        }
    }

    size_t chunk_size_;
    size_t chunk_count_;
    uint8_t* pool_;
    FreeNode* free_head_;
    size_t allocated_count_;
};

struct TestObject {
    int x;
    double y;
    char data[48];
};

template <typename Func>
static double measure_ns(Func&& func) {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::nano>(end - start).count();
}

static void benchmark_pool(size_t iterations) {
    MemoryPool pool(sizeof(TestObject), iterations);
    std::vector<void*> ptrs(iterations);

    double alloc_time = measure_ns([&] {
        for (size_t i = 0; i < iterations; ++i) {
            ptrs[i] = pool.allocate();
        }
    });

    double dealloc_time = measure_ns([&] {
        for (size_t i = 0; i < iterations; ++i) {
            pool.deallocate(ptrs[i]);
        }
    });

    // Mixed: allocate all, deallocate half, reallocate half
    double mixed_time = measure_ns([&] {
        for (size_t i = 0; i < iterations; ++i) {
            ptrs[i] = pool.allocate();
        }
        for (size_t i = 0; i < iterations / 2; ++i) {
            pool.deallocate(ptrs[i]);
            ptrs[i] = nullptr;
        }
        for (size_t i = 0; i < iterations / 2; ++i) {
            ptrs[i] = pool.allocate();
        }
        for (size_t i = 0; i < iterations; ++i) {
            pool.deallocate(ptrs[i]);
        }
    });

    std::cout << "  Pool Allocator:\n"
              << "    Allocate   " << iterations << " objects: "
              << std::fixed << std::setprecision(2) << alloc_time / 1e6 << " ms ("
              << alloc_time / iterations << " ns/op)\n"
              << "    Deallocate " << iterations << " objects: "
              << dealloc_time / 1e6 << " ms ("
              << dealloc_time / iterations << " ns/op)\n"
              << "    Mixed workload:              "
              << mixed_time / 1e6 << " ms\n";
}

static void benchmark_standard(size_t iterations) {
    std::vector<TestObject*> ptrs(iterations);

    double alloc_time = measure_ns([&] {
        for (size_t i = 0; i < iterations; ++i) {
            ptrs[i] = new TestObject;
        }
    });

    double dealloc_time = measure_ns([&] {
        for (size_t i = 0; i < iterations; ++i) {
            delete ptrs[i];
        }
    });

    double mixed_time = measure_ns([&] {
        for (size_t i = 0; i < iterations; ++i) {
            ptrs[i] = new TestObject;
        }
        for (size_t i = 0; i < iterations / 2; ++i) {
            delete ptrs[i];
            ptrs[i] = nullptr;
        }
        for (size_t i = 0; i < iterations / 2; ++i) {
            ptrs[i] = new TestObject;
        }
        for (size_t i = 0; i < iterations; ++i) {
            delete ptrs[i];
        }
    });

    std::cout << "  Standard new/delete:\n"
              << "    Allocate   " << iterations << " objects: "
              << std::fixed << std::setprecision(2) << alloc_time / 1e6 << " ms ("
              << alloc_time / iterations << " ns/op)\n"
              << "    Deallocate " << iterations << " objects: "
              << dealloc_time / 1e6 << " ms ("
              << dealloc_time / iterations << " ns/op)\n"
              << "    Mixed workload:              "
              << mixed_time / 1e6 << " ms\n";
}

static void run_correctness_tests() {
    std::cout << "=== Correctness Tests ===\n";

    {
        MemoryPool pool(sizeof(int), 10);
        assert(pool.available() == 10);
        assert(pool.allocated() == 0);

        void* p1 = pool.allocate();
        assert(pool.allocated() == 1);
        assert(pool.available() == 9);

        void* p2 = pool.allocate();
        assert(p1 != p2);

        pool.deallocate(p1);
        assert(pool.allocated() == 1);
        assert(pool.available() == 9);

        void* p3 = pool.allocate();
        assert(p3 == p1);

        pool.deallocate(p2);
        pool.deallocate(p3);
        assert(pool.allocated() == 0);
        std::cout << "  Basic alloc/dealloc: PASS\n";
    }

    {
        const size_t count = 100;
        MemoryPool pool(sizeof(double), count);
        std::vector<void*> ptrs;
        for (size_t i = 0; i < count; ++i) {
            ptrs.push_back(pool.allocate());
        }
        assert(pool.available() == 0);

        bool threw = false;
        try {
            pool.allocate();
        } catch (const std::bad_alloc&) {
            threw = true;
        }
        assert(threw);

        for (auto* p : ptrs) pool.deallocate(p);
        assert(pool.available() == count);
        std::cout << "  Exhaustion test:     PASS\n";
    }

    {
        MemoryPool pool(1, 1000);
        assert(pool.chunk_size() >= sizeof(void*));
        std::cout << "  Min chunk size:      PASS (chunk_size=" << pool.chunk_size() << ")\n";
    }

    std::cout << "All correctness tests passed.\n\n";
}

int main() {
    run_correctness_tests();

    const std::vector<size_t> sizes = {1000, 10000, 100000, 1000000};

    std::cout << "=== Performance Benchmarks ===\n"
              << "Object size: " << sizeof(TestObject) << " bytes\n\n";

    for (size_t n : sizes) {
        std::cout << "--- " << n << " allocations ---\n";
        benchmark_pool(n);
        benchmark_standard(n);
        std::cout << '\n';
    }

    return 0;
}
