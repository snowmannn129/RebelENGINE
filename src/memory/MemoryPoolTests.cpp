#include <gtest/gtest.h>
#include "core/MemoryPool.h"
#include <thread>
#include <vector>

using namespace RebelCAD::Core;

// Test structure to verify memory alignment and initialization
struct TestStruct {
    int x;
    double y;
    char z;
    TestStruct() : x(0), y(0.0), z('a') {}
};

TEST(MemoryPoolTests, BasicAllocationDeallocation) {
    MemoryPool<TestStruct, 4> pool;
    
    // Verify initial state
    EXPECT_EQ(pool.getActiveAllocations(), 0);
    EXPECT_EQ(pool.getCapacity(), 4);

    // Test basic allocation
    auto ptr1 = pool.allocate();
    EXPECT_NE(ptr1, nullptr);
    EXPECT_EQ(pool.getActiveAllocations(), 1);

    // Test deallocation
    pool.deallocate(ptr1);
    EXPECT_EQ(pool.getActiveAllocations(), 0);
}

TEST(MemoryPoolTests, MultipleAllocations) {
    MemoryPool<TestStruct, 4> pool;
    std::vector<TestStruct*> ptrs;

    // Allocate all available slots
    for (int i = 0; i < 4; ++i) {
        ptrs.push_back(pool.allocate());
        EXPECT_EQ(pool.getActiveAllocations(), i + 1);
    }

    // Verify all pointers are unique
    for (size_t i = 0; i < ptrs.size(); ++i) {
        for (size_t j = i + 1; j < ptrs.size(); ++j) {
            EXPECT_NE(ptrs[i], ptrs[j]);
        }
    }

    // Test automatic block expansion
    auto extraPtr = pool.allocate();
    EXPECT_NE(extraPtr, nullptr);
    EXPECT_EQ(pool.getCapacity(), 8); // Should have doubled

    // Cleanup
    for (auto ptr : ptrs) {
        pool.deallocate(ptr);
    }
    pool.deallocate(extraPtr);
}

TEST(MemoryPoolTests, ThreadSafety) {
    MemoryPool<TestStruct, 1000> pool;
    constexpr int numThreads = 4;
    constexpr int allocsPerThread = 100;
    
    std::vector<std::thread> threads;
    std::atomic<int> successCount{0};

    auto threadFunc = [&]() {
        try {
            std::vector<TestStruct*> threadPtrs;
            
            // Allocate
            for (int i = 0; i < allocsPerThread; ++i) {
                threadPtrs.push_back(pool.allocate());
            }

            // Small delay to increase chance of thread interleaving
            std::this_thread::sleep_for(std::chrono::milliseconds(1));

            // Deallocate
            for (auto ptr : threadPtrs) {
                pool.deallocate(ptr);
            }

            successCount++;
        } catch (const std::exception& e) {
            // Test will fail if any thread throws
            FAIL() << "Thread exception: " << e.what();
        }
    };

    // Start threads
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(threadFunc);
    }

    // Wait for completion
    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(successCount, numThreads);
    EXPECT_EQ(pool.getActiveAllocations(), 0);
}

TEST(MemoryPoolTests, ErrorHandling) {
    MemoryPool<TestStruct, 4> pool;
    
    // Test null pointer deallocation
    EXPECT_THROW(pool.deallocate(nullptr), std::runtime_error);

    // Test invalid pointer deallocation
    TestStruct invalidObj;
    EXPECT_THROW(pool.deallocate(&invalidObj), std::runtime_error);

    // Test memory leak detection
    {
        MemoryPool<TestStruct, 4> tempPool;
        auto ptr = tempPool.allocate();
        // Deliberately not deallocating to test destructor assertion
        // Note: This would trigger an assertion in debug builds
    }
}
