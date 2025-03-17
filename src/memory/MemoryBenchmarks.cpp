#include <benchmark/benchmark.h>
#include "core/BlockAllocator.h"
#include <vector>
#include <random>
#include <thread>

using namespace RebelCAD::Core;

// Benchmark small allocations using fast path
static void BM_BlockAllocator_FastPath(benchmark::State& state) {
    const size_t POOL_SIZE = 1024 * 1024;
    BlockAllocator allocator(POOL_SIZE);
    std::vector<void*> ptrs;
    ptrs.reserve(1000);
    
    for (auto _ : state) {
        // Small allocations should use fast path
        for (int i = 0; i < 1000; ++i) {
            void* ptr = allocator.allocate(64); // Below SMALL_ALLOCATION_THRESHOLD
            ptrs.push_back(ptr);
        }
        
        // Deallocate
        for (void* ptr : ptrs) {
            allocator.deallocate(ptr);
        }
        ptrs.clear();
    }
}
BENCHMARK(BM_BlockAllocator_FastPath)
    ->Unit(benchmark::kMicrosecond)
    ->UseRealTime();

// Benchmark segmented allocation performance
static void BM_BlockAllocator_SegmentedAllocation(benchmark::State& state) {
    const size_t POOL_SIZE = 1024 * 1024;
    BlockAllocator allocator(POOL_SIZE);
    std::vector<void*> ptrs;
    ptrs.reserve(300);
    
    for (auto _ : state) {
        // Allocate across different segments
        for (int i = 0; i < 100; ++i) {
            ptrs.push_back(allocator.allocate(64));   // Small segment
            ptrs.push_back(allocator.allocate(256));  // Medium segment
            ptrs.push_back(allocator.allocate(1024)); // Large segment
        }
        
        // Deallocate in mixed order
        while (!ptrs.empty()) {
            allocator.deallocate(ptrs.back());
            ptrs.pop_back();
        }
    }
}
BENCHMARK(BM_BlockAllocator_SegmentedAllocation)
    ->Unit(benchmark::kMicrosecond)
    ->UseRealTime();

// Benchmark high concurrency performance
static void BM_BlockAllocator_Concurrency(benchmark::State& state) {
    const size_t POOL_SIZE = 1024 * 1024 * 10; // 10MB for multiple threads
    static BlockAllocator allocator(POOL_SIZE);
    
    for (auto _ : state) {
        std::vector<std::thread> threads;
        const int numThreads = state.range(0);
        
        for (int t = 0; t < numThreads; ++t) {
            threads.emplace_back([&]() {
                std::vector<void*> threadPtrs;
                threadPtrs.reserve(100);
                
                for (int i = 0; i < 100; ++i) {
                    void* ptr = allocator.allocate(64);
                    threadPtrs.push_back(ptr);
                    
                    if (i % 2 == 0 && !threadPtrs.empty()) {
                        allocator.deallocate(threadPtrs.back());
                        threadPtrs.pop_back();
                    }
                }
                
                for (void* ptr : threadPtrs) {
                    allocator.deallocate(ptr);
                }
            });
        }
        
        for (auto& thread : threads) {
            thread.join();
        }
    }
}
BENCHMARK(BM_BlockAllocator_Concurrency)
    ->RangeMultiplier(2)
    ->Range(1, 16)
    ->Unit(benchmark::kMicrosecond)
    ->UseRealTime();

// Benchmark fragmentation and compaction
static void BM_BlockAllocator_Fragmentation(benchmark::State& state) {
    const size_t POOL_SIZE = 1024 * 1024;
    BlockAllocator allocator(POOL_SIZE);
    std::vector<void*> ptrs;
    ptrs.reserve(1000);
    
    for (auto _ : state) {
        state.PauseTiming();
        // Create fragmentation pattern
        for (int i = 0; i < 1000; ++i) {
            void* ptr = allocator.allocate(64);
            ptrs.push_back(ptr);
            
            if (i % 2 == 0) {
                allocator.deallocate(ptrs[i]);
                ptrs[i] = nullptr;
            }
        }
        state.ResumeTiming();
        
        // Measure compaction performance
        void* largePtr = allocator.allocate(512); // Should trigger compaction
        
        state.PauseTiming();
        // Cleanup
        if (largePtr) allocator.deallocate(largePtr);
        for (void* ptr : ptrs) {
            if (ptr) allocator.deallocate(ptr);
        }
        ptrs.clear();
        state.ResumeTiming();
    }
}
BENCHMARK(BM_BlockAllocator_Fragmentation)
    ->Unit(benchmark::kMicrosecond)
    ->UseRealTime();

// Benchmark mixed allocation patterns
static void BM_BlockAllocator_MixedAllocation(benchmark::State& state) {
    const size_t POOL_SIZE = 1024 * 1024;
    BlockAllocator allocator(POOL_SIZE);
    std::vector<std::pair<void*, size_t>> allocations;
    allocations.reserve(1000);
    std::mt19937 gen(42);
    std::uniform_int_distribution<> sizeDist(8, 2048);
    
    for (auto _ : state) {
        for (int i = 0; i < 1000; ++i) {
            size_t size = sizeDist(gen);
            void* ptr = allocator.allocate(size);
            if (ptr) allocations.emplace_back(ptr, size);
            
            if (i % 3 == 0 && !allocations.empty()) {
                size_t index = gen() % allocations.size();
                allocator.deallocate(allocations[index].first);
                allocations[index] = allocations.back();
                allocations.pop_back();
            }
        }
        
        for (const auto& [ptr, size] : allocations) {
            allocator.deallocate(ptr);
        }
        allocations.clear();
    }
}
BENCHMARK(BM_BlockAllocator_MixedAllocation)
    ->Unit(benchmark::kMicrosecond)
    ->UseRealTime();

// Benchmark allocation size impact
static void BM_BlockAllocator_SizeImpact(benchmark::State& state) {
    const size_t POOL_SIZE = 1024 * 1024;
    BlockAllocator allocator(POOL_SIZE);
    std::vector<void*> ptrs;
    ptrs.reserve(1000);
    
    const size_t allocationSize = state.range(0);
    
    for (auto _ : state) {
        for (int i = 0; i < 1000; ++i) {
            void* ptr = allocator.allocate(allocationSize);
            ptrs.push_back(ptr);
        }
        
        for (void* ptr : ptrs) {
            allocator.deallocate(ptr);
        }
        ptrs.clear();
    }
}
BENCHMARK(BM_BlockAllocator_SizeImpact)
    ->RangeMultiplier(4)
    ->Range(8, 8<<10)
    ->Unit(benchmark::kMicrosecond)
    ->UseRealTime();

// Compare with standard allocator
static void BM_StandardAllocator(benchmark::State& state) {
    std::vector<void*> ptrs;
    ptrs.reserve(1000);
    
    for (auto _ : state) {
        for (int i = 0; i < 1000; ++i) {
            void* ptr = std::malloc(64);
            ptrs.push_back(ptr);
        }
        
        for (void* ptr : ptrs) {
            std::free(ptr);
        }
        ptrs.clear();
    }
}
BENCHMARK(BM_StandardAllocator)
    ->Unit(benchmark::kMicrosecond)
    ->UseRealTime();

BENCHMARK_MAIN();
