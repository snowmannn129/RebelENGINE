#pragma once

#include <cstddef>
#include <mutex>
#include <atomic>
#include <cassert>
#include <vector>
#include <memory>
#include <algorithm>
#include <array>
#include <shared_mutex>

namespace RebelCAD {
namespace Core {

/**
 * @brief A memory block allocator that efficiently manages variable-sized memory blocks.
 * 
 * The BlockAllocator provides:
 * - Variable block size allocation with cache-friendly alignment
 * - Memory alignment control and optimization
 * - Block coalescing to reduce fragmentation
 * - Thread-safe operations with minimal contention using segmented locks
 * - Memory leak detection and performance monitoring
 * - Fragmentation tracking and management
 * - Proactive memory compaction
 */
class BlockAllocator {
public:
    /**
     * @brief Constructs a BlockAllocator with specified initial capacity.
     * @param initialCapacity Initial memory pool size in bytes
     * @param alignment Memory alignment requirement (default: 16 bytes)
     */
    explicit BlockAllocator(size_t initialCapacity, size_t alignment = 16) 
        : m_alignment(alignment)
        , m_totalAllocated(0)
        , m_fragmentationLevel(0)
        , m_allocCount(0)
        , m_deallocCount(0)
        , m_peakMemoryUsage(0)
        , m_totalMemory(initialCapacity)
        , m_compactionThreshold(70) // Start compaction at 70% fragmentation
        , m_segments(NUM_SEGMENTS) {
        // Validate alignment is power of 2
        assert((alignment & (alignment - 1)) == 0 && "Alignment must be power of 2");
        
        // Calculate segment size
        const size_t segmentSize = (initialCapacity + NUM_SEGMENTS - 1) / NUM_SEGMENTS;
        
        // Initialize segments
        for (size_t i = 0; i < NUM_SEGMENTS; ++i) {
            auto& segment = m_segments[i];
            segment.memory = std::make_unique<unsigned char[]>(segmentSize);
            segment.freeList = reinterpret_cast<Block*>(segment.memory.get());
            segment.freeList->size = segmentSize - sizeof(Block);
            segment.freeList->used = false;
            segment.freeList->next = nullptr;
            segment.totalSize = segmentSize;
        }
    }

    /**
     * @brief Destructor that ensures all memory is properly freed.
     */
    ~BlockAllocator() {
        assert(m_totalAllocated == 0 && "Memory leak detected: Not all allocations were freed");
    }

    /**
     * @brief Allocates a block of memory with specified size and alignment.
     * @param size Requested allocation size in bytes
     * @return Pointer to aligned memory block, or nullptr if allocation fails
     */
    void* allocate(size_t size) {
        if (size == 0) return nullptr;
        
        const size_t alignedSize = alignSize(size);
        const size_t segmentIndex = selectSegment(alignedSize);
        auto& segment = m_segments[segmentIndex];
        
        // Try lock-free fast path for small allocations
        if (alignedSize <= SMALL_ALLOCATION_THRESHOLD) {
            if (void* ptr = tryFastPathAllocation(segment, alignedSize)) {
                return ptr;
            }
        }
        
        // Fall back to standard allocation path
        std::unique_lock<std::shared_mutex> lock(segment.mutex);
        
        // Find best fit block
        Block* prev = nullptr;
        Block* curr = segment.freeList;
        Block* bestFit = nullptr;
        Block* bestFitPrev = nullptr;
        
        while (curr) {
            if (!curr->used && curr->size >= alignedSize) {
                if (!bestFit || curr->size < bestFit->size) {
                    bestFit = curr;
                    bestFitPrev = prev;
                }
            }
            prev = curr;
            curr = curr->next;
        }
        
        if (!bestFit) {
            // Try other segments if allocation fails
            lock.unlock();
            return allocateFromAnySegment(alignedSize);
        }
        
        // Apply smart splitting strategy
        if (shouldSplitBlock(bestFit->size, alignedSize)) {
            splitBlock(bestFit, alignedSize);
        }
        
        bestFit->used = true;
        m_totalAllocated += bestFit->size;
        m_allocCount++;
        
        // Update peak memory usage
        m_peakMemoryUsage = std::max(m_peakMemoryUsage.load(), m_totalAllocated.load());
        
        // Check fragmentation and trigger compaction if needed
        if (shouldCompact(segment)) {
            // Release lock and schedule async compaction
            lock.unlock();
            scheduleCompaction(segmentIndex);
        }
        
        return reinterpret_cast<void*>(reinterpret_cast<unsigned char*>(bestFit) + sizeof(Block));
    }

    /**
     * @brief Deallocates a previously allocated memory block.
     * @param ptr Pointer to the memory block to deallocate
     */
    void deallocate(void* ptr) {
        if (!ptr) return;
        
        // Get block header and segment
        Block* block = reinterpret_cast<Block*>(
            reinterpret_cast<unsigned char*>(ptr) - sizeof(Block)
        );
        const size_t segmentIndex = getSegmentIndex(block);
        auto& segment = m_segments[segmentIndex];
        
        // Try lock-free fast path for small deallocations
        if (block->size <= SMALL_ALLOCATION_THRESHOLD) {
            if (tryFastPathDeallocation(block, segment)) {
                return;
            }
        }
        
        // Fall back to standard deallocation path
        std::unique_lock<std::shared_mutex> lock(segment.mutex);
        
        // Validate block
        assert(block->used && "Double free detected");
        assert(isBlockInSegment(block, segment));
        
        block->used = false;
        m_totalAllocated -= block->size;
        m_deallocCount++;
        
        // Coalesce with adjacent free blocks
        coalesceBlocks(segment);
        
        // Update fragmentation level and check for compaction
        if (shouldCompact(segment)) {
            lock.unlock();
            scheduleCompaction(segmentIndex);
        }
    }

    /**
     * @brief Returns the total amount of allocated memory.
     * @return Total allocated bytes
     */
    size_t getAllocatedSize() const {
        return m_totalAllocated.load();
    }

    /**
     * @brief Returns the total capacity of the allocator.
     * @return Total capacity in bytes
     */
    size_t getCapacity() const {
        return m_totalMemory;
    }

    /**
     * @brief Returns the current fragmentation level.
     * @return Fragmentation level as a percentage (0-100)
     */
    size_t getFragmentationLevel() const {
        return m_fragmentationLevel.load();
    }

    /**
     * @brief Returns performance statistics for the allocator.
     * @return Tuple of (allocation count, deallocation count, peak memory usage)
     */
    std::tuple<size_t, size_t, size_t> getPerformanceStats() const {
        return {
            m_allocCount.load(),
            m_deallocCount.load(),
            m_peakMemoryUsage.load()
        };
    }

private:
    static constexpr size_t NUM_SEGMENTS = 16; // Power of 2 for efficient indexing
    static constexpr size_t SEGMENT_MASK = NUM_SEGMENTS - 1;
    static constexpr size_t SMALL_ALLOCATION_THRESHOLD = 128; // Size threshold for fast path
    static constexpr size_t MIN_SPLIT_THRESHOLD = 32; // Minimum size to consider splitting
    static constexpr double SPLIT_RATIO_THRESHOLD = 0.75; // Split if remaining size ratio > threshold

    /**
     * @brief Memory block header structure aligned for optimal cache usage.
     */
    struct alignas(64) Block {
        std::atomic<size_t> size;  ///< Size of the block's payload
        std::atomic<bool> used;    ///< Whether block is currently allocated
        Block* next;               ///< Pointer to next block in list
        uint8_t padding[40];       ///< Padding to ensure 64-byte alignment
    };

    /**
     * @brief Segment structure for partitioned memory management.
     */
    struct Segment {
        std::unique_ptr<unsigned char[]> memory;
        Block* freeList;
        size_t totalSize;
        std::shared_mutex mutex;
        std::atomic<size_t> fragmentationLevel{0};
    };

    /**
     * @brief Selects the appropriate segment for an allocation.
     * @param size Size of the allocation
     * @return Index of the selected segment
     */
    size_t selectSegment(size_t size) const {
        // Use size-based distribution for better load balancing
        return (size / SMALL_ALLOCATION_THRESHOLD) & SEGMENT_MASK;
    }

    /**
     * @brief Gets the segment index for a block.
     * @param block Pointer to the block
     * @return Index of the segment containing the block
     */
    size_t getSegmentIndex(Block* block) const {
        uintptr_t blockAddr = reinterpret_cast<uintptr_t>(block);
        for (size_t i = 0; i < NUM_SEGMENTS; ++i) {
            uintptr_t segmentStart = reinterpret_cast<uintptr_t>(m_segments[i].memory.get());
            uintptr_t segmentEnd = segmentStart + m_segments[i].totalSize;
            if (blockAddr >= segmentStart && blockAddr < segmentEnd) {
                return i;
            }
        }
        assert(false && "Block not found in any segment");
        return 0;
    }

    /**
     * @brief Attempts fast-path allocation for small blocks.
     * @param segment Target segment
     * @param size Requested size
     * @return Allocated pointer or nullptr if fast path fails
     */
    void* tryFastPathAllocation(Segment& segment, size_t size) {
        // Try to find a free block without locking
        Block* curr = segment.freeList;
        while (curr) {
            bool expected = false;
            if (!curr->used.load(std::memory_order_relaxed) &&
                curr->size.load(std::memory_order_relaxed) >= size &&
                curr->used.compare_exchange_strong(expected, true)) {
                m_totalAllocated += size;
                m_allocCount++;
                return reinterpret_cast<void*>(reinterpret_cast<unsigned char*>(curr) + sizeof(Block));
            }
            curr = curr->next;
        }
        return nullptr;
    }

    /**
     * @brief Attempts fast-path deallocation for small blocks.
     * @param block Block to deallocate
     * @param segment Target segment
     * @return True if fast path succeeds
     */
    bool tryFastPathDeallocation(Block* block, Segment& segment) {
        bool expected = true;
        if (block->used.compare_exchange_strong(expected, false)) {
            m_totalAllocated -= block->size;
            m_deallocCount++;
            return true;
        }
        return false;
    }

    /**
     * @brief Allocates from any available segment.
     * @param size Requested size
     * @return Allocated pointer or nullptr if all segments are full
     */
    void* allocateFromAnySegment(size_t size) {
        for (size_t i = 0; i < NUM_SEGMENTS; ++i) {
            auto& segment = m_segments[i];
            std::unique_lock<std::shared_mutex> lock(segment.mutex);
            
            Block* curr = segment.freeList;
            while (curr) {
                if (!curr->used && curr->size >= size) {
                    curr->used = true;
                    m_totalAllocated += size;
                    m_allocCount++;
                    return reinterpret_cast<void*>(reinterpret_cast<unsigned char*>(curr) + sizeof(Block));
                }
                curr = curr->next;
            }
        }
        return nullptr;
    }

    /**
     * @brief Determines if a block should be split based on size and policy.
     * @param blockSize Size of the current block
     * @param requestedSize Requested allocation size
     * @return True if block should be split
     */
    bool shouldSplitBlock(size_t blockSize, size_t requestedSize) const {
        if (blockSize - requestedSize < MIN_SPLIT_THRESHOLD) return false;
        return static_cast<double>(requestedSize) / blockSize <= SPLIT_RATIO_THRESHOLD;
    }

    /**
     * @brief Splits a block into two parts.
     * @param block Block to split
     * @param size Size for the first part
     */
    void splitBlock(Block* block, size_t size) {
        Block* newBlock = reinterpret_cast<Block*>(
            reinterpret_cast<unsigned char*>(block) + sizeof(Block) + size
        );
        newBlock->size = block->size - size - sizeof(Block);
        newBlock->used = false;
        newBlock->next = block->next;
        
        block->size = size;
        block->next = newBlock;
    }

    /**
     * @brief Checks if compaction should be triggered.
     * @param segment Segment to check
     * @return True if compaction should be performed
     */
    bool shouldCompact(const Segment& segment) const {
        return segment.fragmentationLevel.load() > m_compactionThreshold;
    }

    /**
     * @brief Schedules asynchronous compaction for a segment.
     * @param segmentIndex Index of segment to compact
     */
    void scheduleCompaction(size_t segmentIndex) {
        // In a real implementation, this would queue the compaction
        // task to a background thread. For now, we do it immediately.
        compactSegment(segmentIndex);
    }

    /**
     * @brief Compacts a memory segment to reduce fragmentation.
     * @param segmentIndex Index of segment to compact
     */
    void compactSegment(size_t segmentIndex) {
        auto& segment = m_segments[segmentIndex];
        std::unique_lock<std::shared_mutex> lock(segment.mutex);
        
        // Collect all used blocks
        std::vector<std::pair<Block*, size_t>> usedBlocks;
        Block* curr = segment.freeList;
        while (curr) {
            if (curr->used) {
                usedBlocks.emplace_back(curr, curr->size);
            }
            curr = curr->next;
        }
        
        // Sort blocks by address for sequential placement
        std::sort(usedBlocks.begin(), usedBlocks.end());
        
        // Reset segment
        segment.freeList = reinterpret_cast<Block*>(segment.memory.get());
        Block* current = segment.freeList;
        
        // Rebuild block list with compacted blocks
        for (const auto& [block, size] : usedBlocks) {
            if (current != block) {
                std::memcpy(current, block, sizeof(Block) + size);
            }
            current = reinterpret_cast<Block*>(
                reinterpret_cast<unsigned char*>(current) + sizeof(Block) + size
            );
        }
        
        // Create final free block
        current->size = reinterpret_cast<unsigned char*>(segment.memory.get()) +
                       segment.totalSize - reinterpret_cast<unsigned char*>(current) -
                       sizeof(Block);
        current->used = false;
        current->next = nullptr;
        
        // Update fragmentation level
        updateFragmentationLevel(segment);
    }

    /**
     * @brief Updates the fragmentation level for a segment.
     * @param segment Segment to update
     */
    void updateFragmentationLevel(Segment& segment) {
        size_t totalFreeSpace = 0;
        size_t freeBlockCount = 0;
        
        Block* curr = segment.freeList;
        while (curr) {
            if (!curr->used) {
                totalFreeSpace += curr->size;
                freeBlockCount++;
            }
            curr = curr->next;
        }
        
        if (totalFreeSpace > 0) {
            size_t avgBlockSize = totalFreeSpace / freeBlockCount;
            size_t idealBlockSize = totalFreeSpace;
            segment.fragmentationLevel = static_cast<size_t>(
                (1.0 - static_cast<double>(avgBlockSize) / idealBlockSize) * 100
            );
        } else {
            segment.fragmentationLevel = 0;
        }
        
        // Update global fragmentation level
        updateGlobalFragmentationLevel();
    }

    /**
     * @brief Updates the global fragmentation level based on all segments.
     */
    void updateGlobalFragmentationLevel() {
        size_t totalFragmentation = 0;
        for (const auto& segment : m_segments) {
            totalFragmentation += segment.fragmentationLevel;
        }
        m_fragmentationLevel = totalFragmentation / NUM_SEGMENTS;
    }

    /**
     * @brief Coalesces adjacent free blocks in a segment.
     * @param segment Segment to coalesce
     */
    void coalesceBlocks(Segment& segment) {
        Block* curr = segment.freeList;
        
        while (curr && curr->next) {
            if (!curr->used && !curr->next->used) {
                curr->size += sizeof(Block) + curr->next->size;
                curr->next = curr->next->next;
            } else {
                curr = curr->next;
            }
        }
        
        updateFragmentationLevel(segment);
    }

    /**
     * @brief Checks if a block belongs to a segment.
     * @param block Block to check
     * @param segment Segment to verify against
     * @return True if block belongs to segment
     */
    bool isBlockInSegment(Block* block, const Segment& segment) const {
        unsigned char* blockAddr = reinterpret_cast<unsigned char*>(block);
        return blockAddr >= segment.memory.get() &&
               blockAddr < segment.memory.get() + segment.totalSize;
    }

    /**
     * @brief Aligns the requested size to the allocator's alignment requirement.
     * @param size Size to align
     * @return Aligned size
     */
    size_t alignSize(size_t size) const {
        return (size + m_alignment - 1) & ~(m_alignment - 1);
    }

    std::vector<Segment> m_segments;              ///< Memory segments
    const size_t m_alignment;                     ///< Memory alignment requirement
    std::atomic<size_t> m_totalAllocated;        ///< Total allocated memory
    std::atomic<size_t> m_fragmentationLevel;    ///< Current fragmentation level
    std::atomic<size_t> m_allocCount;            ///< Total number of allocations
    std::atomic<size_t> m_deallocCount;          ///< Total number of deallocations
    std::atomic<size_t> m_peakMemoryUsage;       ///< Peak memory usage
    const size_t m_totalMemory;                  ///< Total memory pool size
    const size_t m_compactionThreshold;          ///< Fragmentation threshold for compaction
};

} // namespace Core
} // namespace RebelCAD
