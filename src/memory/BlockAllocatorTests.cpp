#include <gtest/gtest.h>
#include "core/BlockAllocator.h"
#include <thread>
#include <vector>
#include <random>
#include <future>

using namespace RebelCAD::Core;

class BlockAllocatorTest : public ::testing::Test {
protected:
    const size_t TEST_POOL_SIZE = 1024 * 1024; // 1MB
    const size_t DEFAULT_ALIGNMENT = 16;
    
    std::unique_ptr<BlockAllocator> allocator;
    
    void SetUp() override {
        allocator = std::make_unique<BlockAllocator>(TEST_POOL_SIZE, DEFAULT_ALIGNMENT);
    }
};

TEST_F(BlockAllocatorTest, FastPathAllocation) {
    // Test small allocations that should use fast path
    std::vector<void*> ptrs;
    const int numAllocs = 1000;
    
    // Allocate small blocks that should use fast path
    for (int i = 0; i < numAllocs; ++i) {
        void* ptr = allocator->allocate(64); // Below SMALL_ALLOCATION_THRESHOLD
        ASSERT_NE(ptr, nullptr);
        ptrs.push_back(ptr);
    }
    
    // Verify allocations
    EXPECT_EQ(allocator->getAllocatedSize(), numAllocs * 64);
    
    // Deallocate
    for (void* ptr : ptrs) {
        allocator->deallocate(ptr);
    }
    
    EXPECT_EQ(allocator->getAllocatedSize(), 0);
}

TEST_F(BlockAllocatorTest, SegmentedAllocation) {
    // Test allocations across different segments
    std::vector<void*> smallPtrs;
    std::vector<void*> mediumPtrs;
    std::vector<void*> largePtrs;
    
    // Allocate blocks of different sizes
    for (int i = 0; i < 100; ++i) {
        void* small = allocator->allocate(64);    // Small segment
        void* medium = allocator->allocate(256);  // Medium segment
        void* large = allocator->allocate(1024);  // Large segment
        
        ASSERT_NE(small, nullptr);
        ASSERT_NE(medium, nullptr);
        ASSERT_NE(large, nullptr);
        
        smallPtrs.push_back(small);
        mediumPtrs.push_back(medium);
        largePtrs.push_back(large);
    }
    
    // Deallocate in mixed order
    for (void* ptr : mediumPtrs) allocator->deallocate(ptr);
    for (void* ptr : largePtrs) allocator->deallocate(ptr);
    for (void* ptr : smallPtrs) allocator->deallocate(ptr);
    
    EXPECT_EQ(allocator->getAllocatedSize(), 0);
}

TEST_F(BlockAllocatorTest, SmartBlockSplitting) {
    // Test block splitting strategy
    void* ptr1 = allocator->allocate(256);
    ASSERT_NE(ptr1, nullptr);
    
    // Allocate a smaller block that should trigger splitting
    void* ptr2 = allocator->allocate(64);
    ASSERT_NE(ptr2, nullptr);
    
    // Verify alignment
    uintptr_t addr1 = reinterpret_cast<uintptr_t>(ptr1);
    uintptr_t addr2 = reinterpret_cast<uintptr_t>(ptr2);
    EXPECT_EQ(addr1 % DEFAULT_ALIGNMENT, 0);
    EXPECT_EQ(addr2 % DEFAULT_ALIGNMENT, 0);
    
    allocator->deallocate(ptr1);
    allocator->deallocate(ptr2);
}

TEST_F(BlockAllocatorTest, HighConcurrencyStress) {
    const int numThreads = 8;
    const int numAllocsPerThread = 10000;
    std::vector<std::future<void>> futures;
    
    auto threadFunc = [this, numAllocsPerThread]() {
        std::vector<void*> threadPtrs;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> sizeDist(16, 1024);
        std::uniform_real_distribution<> opDist(0.0, 1.0);
        
        for (int i = 0; i < numAllocsPerThread; ++i) {
            if (opDist(gen) < 0.7 || threadPtrs.empty()) {
                size_t size = sizeDist(gen);
                void* ptr = allocator->allocate(size);
                if (ptr) threadPtrs.push_back(ptr);
            } else {
                size_t index = gen() % threadPtrs.size();
                allocator->deallocate(threadPtrs[index]);
                threadPtrs[index] = threadPtrs.back();
                threadPtrs.pop_back();
            }
        }
        
        // Cleanup
        for (void* ptr : threadPtrs) {
            allocator->deallocate(ptr);
        }
    };
    
    // Launch threads
    for (int i = 0; i < numThreads; ++i) {
        futures.push_back(std::async(std::launch::async, threadFunc));
    }
    
    // Wait for completion
    for (auto& future : futures) {
        future.get();
    }
    
    // Verify cleanup
    EXPECT_EQ(allocator->getAllocatedSize(), 0);
}

TEST_F(BlockAllocatorTest, FragmentationAndCompaction) {
    std::vector<void*> ptrs;
    const int numAllocs = 1000;
    
    // Create fragmentation pattern
    for (int i = 0; i < numAllocs; ++i) {
        void* ptr = allocator->allocate(64);
        ASSERT_NE(ptr, nullptr);
        ptrs.push_back(ptr);
        
        if (i % 2 == 0) {
            allocator->deallocate(ptrs[i]);
            ptrs[i] = nullptr;
        }
    }
    
    // Record fragmentation level
    size_t fragLevel = allocator->getFragmentationLevel();
    
    // Allocate large block to trigger compaction
    void* largePtr = allocator->allocate(512);
    ASSERT_NE(largePtr, nullptr);
    
    // Verify fragmentation improved
    EXPECT_LT(allocator->getFragmentationLevel(), fragLevel);
    
    // Cleanup
    allocator->deallocate(largePtr);
    for (void* ptr : ptrs) {
        if (ptr) allocator->deallocate(ptr);
    }
}

TEST_F(BlockAllocatorTest, MixedSizeAllocations) {
    std::vector<std::pair<void*, size_t>> allocations;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> sizeDist(8, 2048);
    
    // Perform mixed size allocations
    for (int i = 0; i < 1000; ++i) {
        size_t size = sizeDist(gen);
        void* ptr = allocator->allocate(size);
        ASSERT_NE(ptr, nullptr);
        allocations.emplace_back(ptr, size);
        
        // Randomly deallocate some allocations
        if (i % 3 == 0 && !allocations.empty()) {
            size_t index = gen() % allocations.size();
            allocator->deallocate(allocations[index].first);
            allocations[index] = allocations.back();
            allocations.pop_back();
        }
    }
    
    // Cleanup remaining allocations
    for (const auto& [ptr, size] : allocations) {
        allocator->deallocate(ptr);
    }
    
    EXPECT_EQ(allocator->getAllocatedSize(), 0);
}

TEST_F(BlockAllocatorTest, PerformanceStats) {
    const int numOps = 1000;
    std::vector<void*> ptrs;
    
    // Perform allocations and track stats
    for (int i = 0; i < numOps; ++i) {
        void* ptr = allocator->allocate(128);
        ASSERT_NE(ptr, nullptr);
        ptrs.push_back(ptr);
    }
    
    auto [allocCount, deallocCount, peakUsage] = allocator->getPerformanceStats();
    EXPECT_EQ(allocCount, numOps);
    EXPECT_EQ(deallocCount, 0);
    EXPECT_GE(peakUsage, numOps * 128);
    
    // Deallocate and verify stats
    for (void* ptr : ptrs) {
        allocator->deallocate(ptr);
    }
    
    auto [finalAllocCount, finalDeallocCount, finalPeakUsage] = allocator->getPerformanceStats();
    EXPECT_EQ(finalAllocCount, numOps);
    EXPECT_EQ(finalDeallocCount, numOps);
    EXPECT_GE(finalPeakUsage, numOps * 128);
}

TEST_F(BlockAllocatorTest, AlignmentRequirements) {
    // Test various alignments
    std::vector<size_t> alignments = {16, 32, 64};
    
    for (size_t alignment : alignments) {
        BlockAllocator alignedAllocator(TEST_POOL_SIZE, alignment);
        
        // Allocate blocks of different sizes
        for (size_t size : {32, 64, 128, 256}) {
            void* ptr = alignedAllocator.allocate(size);
            ASSERT_NE(ptr, nullptr);
            
            // Verify alignment
            uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
            EXPECT_EQ(addr % alignment, 0) 
                << "Size " << size << " not aligned to " << alignment << " bytes";
            
            alignedAllocator.deallocate(ptr);
        }
    }
}

TEST_F(BlockAllocatorTest, BoundaryConditions) {
    // Test allocation at pool boundaries
    void* ptr1 = allocator->allocate(TEST_POOL_SIZE / 2);
    ASSERT_NE(ptr1, nullptr);
    
    void* ptr2 = allocator->allocate(TEST_POOL_SIZE / 4);
    ASSERT_NE(ptr2, nullptr);
    
    // This should fail as it exceeds remaining space
    void* ptr3 = allocator->allocate(TEST_POOL_SIZE / 2);
    EXPECT_EQ(ptr3, nullptr);
    
    allocator->deallocate(ptr1);
    allocator->deallocate(ptr2);
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
