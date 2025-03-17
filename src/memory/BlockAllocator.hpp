#pragma once

#include <vector>
#include <cstddef>

namespace rebel_cad {
namespace core {

/**
 * @brief Fixed-size block allocator for efficient memory management
 * 
 * Manages memory in fixed-size blocks, reducing fragmentation and allocation overhead.
 * Uses a free list to track available blocks and grows the pool when needed.
 */
class BlockAllocator {
public:
    /**
     * @brief Construct a new Block Allocator
     * @param blockSize Size of each memory block in bytes
     * @param initialBlocks Initial number of blocks to allocate
     */
    BlockAllocator(size_t blockSize, size_t initialBlocks = 8);

    /**
     * @brief Destroy the Block Allocator and free all memory
     */
    ~BlockAllocator();

    /**
     * @brief Allocate a block of memory
     * @return void* Pointer to allocated block
     */
    void* allocate();

    /**
     * @brief Deallocate a previously allocated block
     * @param ptr Pointer to block to deallocate
     */
    void deallocate(void* ptr);

    /**
     * @brief Get the size of each block
     * @return size_t Block size in bytes
     */
    size_t getBlockSize() const;

    /**
     * @brief Get the total number of blocks
     * @return size_t Total block count
     */
    size_t getNumBlocks() const;

    /**
     * @brief Get the number of free blocks
     * @return size_t Free block count
     */
    size_t getNumFreeBlocks() const;

private:
    /**
     * @brief Add more blocks to the allocator
     * @param count Number of blocks to add
     */
    void addBlocks(size_t count);

    size_t m_blockSize;         ///< Size of each block in bytes
    size_t m_numBlocks;         ///< Total number of blocks
    size_t m_numFreeBlocks;     ///< Number of free blocks
    void* m_firstFreeBlock;     ///< Pointer to first free block
    std::vector<char*> m_blocks; ///< Storage for allocated block arrays

    // Prevent copying
    BlockAllocator(const BlockAllocator&) = delete;
    BlockAllocator& operator=(const BlockAllocator&) = delete;
};

} // namespace core
} // namespace rebel_cad
