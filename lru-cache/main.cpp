#include <iostream>
#include <list>
#include <unordered_map>
#include <string>
#include <chrono>
#include <random>
#include <iomanip>
#include <sstream>

template <typename K, typename V>
class LRUCache {
public:
    explicit LRUCache(size_t capacity) : capacity_(capacity) {}

    bool get(const K& key, V& value) {
        auto it = map_.find(key);
        if (it == map_.end()) {
            ++misses_;
            return false;
        }
        ++hits_;
        items_.splice(items_.begin(), items_, it->second);
        value = it->second->second;
        return true;
    }

    void put(const K& key, const V& value) {
        auto it = map_.find(key);
        if (it != map_.end()) {
            it->second->second = value;
            items_.splice(items_.begin(), items_, it->second);
            return;
        }
        if (items_.size() >= capacity_) {
            auto& back = items_.back();
            map_.erase(back.first);
            items_.pop_back();
        }
        items_.emplace_front(key, value);
        map_[key] = items_.begin();
    }

    bool contains(const K& key) const {
        return map_.find(key) != map_.end();
    }

    size_t size() const { return items_.size(); }
    size_t capacity() const { return capacity_; }
    size_t hits() const { return hits_; }
    size_t misses() const { return misses_; }

    void reset_stats() {
        hits_ = 0;
        misses_ = 0;
    }

    bool erase(const K& key) {
        auto it = map_.find(key);
        if (it == map_.end()) return false;
        items_.erase(it->second);
        map_.erase(it);
        return true;
    }

    void clear() {
        items_.clear();
        map_.clear();
    }

    void print_state(std::ostream& os = std::cout) const {
        os << "Cache [" << items_.size() << "/" << capacity_ << "]: ";
        bool first = true;
        for (auto& [k, v] : items_) {
            if (!first) os << " -> ";
            os << "{" << k << ": " << v << "}";
            first = false;
        }
        os << "\n";
    }

    double hit_rate() const {
        size_t total = hits_ + misses_;
        return total == 0 ? 0.0 : static_cast<double>(hits_) / total * 100.0;
    }

private:
    size_t capacity_;
    std::list<std::pair<K, V>> items_;
    std::unordered_map<K, typename std::list<std::pair<K, V>>::iterator> map_;
    size_t hits_ = 0;
    size_t misses_ = 0;
};

void demo_basic_operations() {
    std::cout << "=== Basic LRU Cache Operations ===\n\n";

    LRUCache<int, std::string> cache(3);

    std::cout << "Inserting: 1->\"one\", 2->\"two\", 3->\"three\"\n";
    cache.put(1, "one");
    cache.put(2, "two");
    cache.put(3, "three");
    cache.print_state();

    std::string val;
    std::cout << "\nAccessing key 1 (should hit): ";
    if (cache.get(1, val))
        std::cout << "HIT -> \"" << val << "\"\n";
    else
        std::cout << "MISS\n";
    cache.print_state();

    std::cout << "\nInserting 4->\"four\" (should evict key 2):\n";
    cache.put(4, "four");
    cache.print_state();

    std::cout << "\nAccessing key 2 (should miss): ";
    if (cache.get(2, val))
        std::cout << "HIT -> \"" << val << "\"\n";
    else
        std::cout << "MISS\n";

    std::cout << "\nUpdating key 3 to \"THREE\":\n";
    cache.put(3, "THREE");
    cache.print_state();

    std::cout << "\nErasing key 4:\n";
    cache.erase(4);
    cache.print_state();

    std::cout << "\nStats: " << cache.hits() << " hits, " << cache.misses()
              << " misses, hit rate: " << std::fixed << std::setprecision(1)
              << cache.hit_rate() << "%\n";
}

void demo_template_types() {
    std::cout << "\n=== Template Flexibility ===\n\n";

    LRUCache<std::string, double> price_cache(4);
    price_cache.put("AAPL", 178.50);
    price_cache.put("GOOG", 141.25);
    price_cache.put("MSFT", 378.91);
    price_cache.put("AMZN", 185.60);
    std::cout << "Stock price cache:\n";
    price_cache.print_state();

    double price;
    price_cache.get("AAPL", price);
    std::cout << "Looked up AAPL: $" << std::fixed << std::setprecision(2) << price << "\n";

    price_cache.put("TSLA", 248.42);
    std::cout << "After adding TSLA (evicts GOOG):\n";
    price_cache.print_state();
}

void benchmark() {
    std::cout << "\n=== Benchmark: LRU Cache vs unordered_map ===\n\n";

    constexpr size_t NUM_KEYS = 10000;
    constexpr size_t CACHE_SIZE = 1000;
    constexpr size_t NUM_OPS = 500000;

    // Zipf-like distribution: most accesses hit a small subset of keys
    std::mt19937 rng(42);
    std::vector<int> access_pattern(NUM_OPS);
    std::geometric_distribution<int> dist(0.005);
    for (auto& key : access_pattern) {
        key = dist(rng) % NUM_KEYS;
    }

    // Benchmark LRU cache
    LRUCache<int, int> lru(CACHE_SIZE);
    auto start = std::chrono::high_resolution_clock::now();
    int lru_val;
    for (int key : access_pattern) {
        if (!lru.get(key, lru_val)) {
            lru.put(key, key * 10);
        }
    }
    auto lru_time = std::chrono::high_resolution_clock::now() - start;

    // Benchmark plain unordered_map (unbounded)
    std::unordered_map<int, int> umap;
    start = std::chrono::high_resolution_clock::now();
    for (int key : access_pattern) {
        auto it = umap.find(key);
        if (it == umap.end()) {
            umap[key] = key * 10;
        }
    }
    auto umap_time = std::chrono::high_resolution_clock::now() - start;

    auto lru_ms = std::chrono::duration<double, std::milli>(lru_time).count();
    auto umap_ms = std::chrono::duration<double, std::milli>(umap_time).count();

    std::cout << "Operations: " << NUM_OPS << ", Keys: " << NUM_KEYS
              << ", Cache size: " << CACHE_SIZE << "\n\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "LRU Cache:      " << lru_ms << " ms | "
              << "Hit rate: " << lru.hit_rate() << "% | "
              << "Memory: " << CACHE_SIZE << " entries (bounded)\n";
    std::cout << "unordered_map:  " << umap_ms << " ms | "
              << "Memory: " << umap.size() << " entries (unbounded)\n";
    std::cout << "\nLRU overhead vs unbounded map: "
              << (lru_ms / umap_ms) << "x slower, but uses "
              << std::setprecision(1)
              << (static_cast<double>(CACHE_SIZE) / umap.size() * 100.0)
              << "% of the memory\n";
}

int main() {
    demo_basic_operations();
    demo_template_types();
    benchmark();
    return 0;
}
